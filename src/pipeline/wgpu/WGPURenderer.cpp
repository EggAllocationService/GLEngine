//
// Created by Kyle Smith on 2026-05-28.
//

#include "pipeline/wgpu/WGPURenderer.h"

#include <cassert>
#include <cstring>
#include <atomic>

#include "glfw3webgpu.h"

extern "C" static void handle_request_adapter(WGPURequestAdapterStatus status,
    WGPUAdapter adapter, WGPUStringView message,
    void* userdata1, void* userdata2) {
    *(WGPUAdapter*)userdata1 = adapter;
}
extern "C" static void handle_request_device(WGPURequestDeviceStatus status,
                                  WGPUDevice device, WGPUStringView message,
                                  void *userdata1, void *userdata2) {
    *(WGPUDevice *)userdata1 = device;

    std::cout << std::string_view(message.data, message.length) << std::endl;

    WGPUSupportedFeatures supportedFeatures;
    wgpuDeviceGetFeatures(device, &supportedFeatures);
    printf("Features enabled: %lu", supportedFeatures.featureCount);

    for (int i = 0; i < supportedFeatures.featureCount; i++) {
        printf("\t 0x%08x \n", supportedFeatures.features[i]);
    }
}

glengine::pipeline::wgpu::WGPURenderer::WGPURenderer(GLFWwindow *window) {
    depthTexture = nullptr;
    surfConfig = WGPU_SURFACE_CONFIGURATION_INIT;

    auto instance = wgpuCreateInstance(nullptr);
    WGPUAdapter adapter = nullptr;
    WGPURequestAdapterOptions options = {
        .nextInChain = nullptr,
        .featureLevel = WGPUFeatureLevel_Core,
        .powerPreference = WGPUPowerPreference_HighPerformance,
        .forceFallbackAdapter = false,
        .backendType = WGPUBackendType_Undefined,
        .compatibleSurface = nullptr
    };

    WGPURequestAdapterCallbackInfo adapterCb = WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT;
    adapterCb.callback = handle_request_adapter;
    adapterCb.userdata1 = &adapter;

    wgpuInstanceRequestAdapter(instance, &options, adapterCb);
    assert(adapter);

    WGPUNativeLimits nativeLimits = {
        .chain = {
            .next = nullptr,
            .sType = static_cast<WGPUSType>(WGPUSType_NativeLimits)
        },
        .maxImmediateSize = 128,
        .maxNonSamplerBindings = WGPU_LIMIT_U32_UNDEFINED,
        .maxBindingArrayElementsPerShaderStage = WGPU_LIMIT_U32_UNDEFINED
    };

    WGPULimits requiredLimits = WGPU_LIMITS_INIT;
    requiredLimits.nextInChain = &nativeLimits.chain;
    requiredLimits.maxImmediateSize = 128;

    auto features = new WGPUNativeFeature[1] { WGPUNativeFeature_Immediates };
    WGPUDeviceDescriptor deviceDescriptor = WGPU_DEVICE_DESCRIPTOR_INIT;
    deviceDescriptor.requiredLimits = &requiredLimits;
    deviceDescriptor.requiredFeatures = reinterpret_cast<WGPUFeatureName*>(&features[0]);
    deviceDescriptor.requiredFeatureCount = 1;

    device = nullptr;

    WGPURequestDeviceCallbackInfo deviceCb = WGPU_REQUEST_DEVICE_CALLBACK_INFO_INIT;
    deviceCb.callback = handle_request_device;
    deviceCb.userdata1 = &device;

    wgpuAdapterRequestDevice(adapter, &deviceDescriptor, deviceCb);
    assert(device);
    queue = wgpuDeviceGetQueue(device);

    delete[] features;

    surface = glfwCreateWindowWGPUSurface(instance, window);

    // find preferred surface texture format (usually the 0th)
    WGPUSurfaceCapabilities caps;
    wgpuSurfaceGetCapabilities(surface, adapter, &caps);
    surfConfig.format = caps.formats[0]; // set preferred format
    surfConfig.usage = WGPUTextureUsage_RenderAttachment;
    surfConfig.presentMode = WGPUPresentMode_Fifo;
    surfConfig.alphaMode = WGPUCompositeAlphaMode_Auto;
    surfConfig.device = device;

    // create universal bind group layout descriptor
    WGPUBindGroupLayoutEntry entry0Desc = {
        .nextInChain = nullptr,
        .binding = 0,
        .visibility = WGPUShaderStage_Fragment | WGPUShaderStage_Vertex,
        .bindingArraySize = 0,
        .buffer = {
            .nextInChain = nullptr,
            .type = WGPUBufferBindingType_Uniform,
            .hasDynamicOffset = false,
            .minBindingSize = sizeof(RenderUniforms),
        },
        .sampler = {},
        .texture = {},
        .storageTexture = {}
    };
    auto universalLayoutDescriptor = WGPUBindGroupLayoutDescriptor {
        .nextInChain = nullptr,
        .label = {
           .data = "Universal Bind Group Layout",
           .length = WGPU_STRLEN
        },
        .entryCount = 1,
        .entries = &entry0Desc
    };
    universalBindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &universalLayoutDescriptor);

    // create render uniforms buffer
    auto renderBufDesc = WGPUBufferDescriptor {
        .nextInChain = nullptr,
        .label = {},
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
        .size = sizeof(RenderUniforms),
        .mappedAtCreation =  false
    };
    renderUniformsBuffer = wgpuDeviceCreateBuffer(device, &renderBufDesc);

    WGPUBindGroupEntry entry0 = WGPU_BIND_GROUP_ENTRY_INIT;
    entry0.buffer = renderUniformsBuffer;
    auto universalDesc = WGPUBindGroupDescriptor {
        .nextInChain = nullptr,
        .label = {},
        .layout = universalBindGroupLayout,
        .entryCount = 1,
        .entries = &entry0
    };
    universalBindGroup = wgpuDeviceCreateBindGroup(device, &universalDesc);

    int2 size;
    glfwGetWindowSize(window, &size.x, &size.y);
    Resize(size);

    buildBuiltinPipelines();
}

WGPUShaderModule glengine::pipeline::wgpu::WGPURenderer::CompileShader(const char* shaders) {
    WGPUShaderSourceWGSL src = {
        .chain = {
            .next = nullptr,
            .sType = WGPUSType_ShaderSourceWGSL
        },
        .code = {
            .data = shaders,
            .length = strlen(shaders)
        }
    };
    WGPUShaderModuleDescriptor desc = {
        .nextInChain = &src.chain,
        .label = {}
    };
    return wgpuDeviceCreateShaderModule(device, &desc);
}

static std::atomic<int> meshIdTracker = 0;

std::shared_ptr<glengine::pipeline::wgpu::GPUMesh> glengine::pipeline::wgpu::WGPURenderer::UploadMesh(const std::vector<Vertex>& vertices) {
    auto bufferDesc = WGPUBufferDescriptor {
        .nextInChain = nullptr,
        .label = {},
        .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
        .size = sizeof(Vertex) * vertices.size(),
        .mappedAtCreation = false
    };
    auto buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
    wgpuQueueWriteBuffer(queue, buffer, 0, vertices.data(), vertices.size() * sizeof(Vertex));

    return std::make_shared<GPUMesh>(buffer, nullptr, vertices.size(), 0, meshIdTracker++);
}

std::shared_ptr<glengine::pipeline::wgpu::GPUMesh> glengine::pipeline::wgpu::WGPURenderer::UploadIndexedMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    auto verticesDesc = WGPUBufferDescriptor{
        .nextInChain = nullptr,
        .label = {},
        .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
        .size = sizeof(Vertex) * vertices.size(),
        .mappedAtCreation = false
    };
    auto vertciesBuf = wgpuDeviceCreateBuffer(device, &verticesDesc);
    wgpuQueueWriteBuffer(queue, vertciesBuf, 0, vertices.data(), vertices.size() * sizeof(Vertex));

    auto indicesDesc = WGPUBufferDescriptor{
        .nextInChain = nullptr,
        .label = {},
        .usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst,
        .size = sizeof(unsigned int) * indices.size(),
        .mappedAtCreation = false
    };
    auto indicesBuf = wgpuDeviceCreateBuffer(device, &indicesDesc);
    wgpuQueueWriteBuffer(queue, indicesBuf, 0, indices.data(), indices.size() * sizeof(unsigned int));

    return std::make_shared<GPUMesh>(vertciesBuf, indicesBuf, vertices.size(), indices.size(), meshIdTracker++);
}


std::shared_ptr<glengine::pipeline::wgpu::RenderPipeline> glengine::pipeline::wgpu::WGPURenderer::
GetRenderPipelineByName(const std::string &name) {
    if (pipelines.contains(name)) {
        return pipelines[name];
    }

    return nullptr;
}

std::shared_ptr<glengine::pipeline::wgpu::RenderPipeline> glengine::pipeline::wgpu::WGPURenderer::BuildRenderPipeline(std::string name,
                                                                                                                      WGPUShaderModule shaders, std::span<WGPUBindGroupLayoutDescriptor> bindGroups, int immediateDataBytes) {

    // manually build pipeline layout
    std::vector<WGPUBindGroupLayout> bindGroupLayouts(bindGroups.size() + 1);

    bindGroupLayouts[0] = universalBindGroupLayout;
    for (int i = 0; i < bindGroups.size(); i++) {
        bindGroupLayouts[i + 1] = wgpuDeviceCreateBindGroupLayout(device, &bindGroups[i]);
    }

    WGPUPipelineLayoutExtras extras = {
        .chain = {
            .next = nullptr,
            .sType = static_cast<WGPUSType>(WGPUSType_PipelineLayoutExtras),
        },
        .immediateDataSize = static_cast<uint32_t>(immediateDataBytes),
    };

    auto layoutDesc = WGPUPipelineLayoutDescriptor {
        .nextInChain = &extras.chain,
        .label = {
           .data = name.data(),
           .length = name.length(),
        },
        .bindGroupLayoutCount = bindGroups.size() + 1,
        .bindGroupLayouts = bindGroupLayouts.data(),
        .immediateSize = static_cast<uint32_t>(immediateDataBytes),
    };

    auto layout = wgpuDeviceCreatePipelineLayout(device, &layoutDesc);


    auto target = WGPUColorTargetState {
        .nextInChain = nullptr,
        .format = surfConfig.format,
        .blend = nullptr,
        .writeMask = WGPUColorWriteMask_All
    };
    auto fragmentState = WGPUFragmentState {
        .nextInChain = nullptr,
        .module = shaders,
        .entryPoint = {
            .data = "fs",
            .length = 2
        },
        .constantCount = 0,
        .constants = nullptr,
        .targetCount = 1,
        .targets = &target
    };
    WGPUVertexAttribute attributes[3] = {
        {
            .nextInChain = nullptr,
            .format = WGPUVertexFormat_Float32x3,
            .offset = offsetof(Vertex, position),
            .shaderLocation = 0
        },
        {
            .nextInChain = nullptr,
            .format = WGPUVertexFormat_Float32x3,
            .offset = offsetof(Vertex, normal),
            .shaderLocation = 1
        },
        {
            .nextInChain = nullptr,
            .format = WGPUVertexFormat_Float32x2,
            .offset = offsetof(Vertex, uv),
            .shaderLocation = 2
        }
    };
    auto vtxLayout = WGPUVertexBufferLayout {
            .nextInChain = nullptr,
            .stepMode = WGPUVertexStepMode_Vertex,
            .arrayStride = sizeof(Vertex),
            .attributeCount = 3,
            .attributes = &attributes[0],
        };
    auto depthStencilState = WGPUDepthStencilState {
        .nextInChain = nullptr,
        .format = WGPUTextureFormat_Depth24Plus,
        .depthWriteEnabled = WGPUOptionalBool_True,
        .depthCompare = WGPUCompareFunction_LessEqual,
        .stencilFront = {},
        .stencilBack = {},
        .stencilReadMask = 0,
        .stencilWriteMask = 0,
        .depthBias = 0,
        .depthBiasSlopeScale = 0,
        .depthBiasClamp = 0
    };


    auto desc = WGPURenderPipelineDescriptor {
        .nextInChain = nullptr,
        .label = {
           .data = name.data(),
           .length = name.length(),
        },
        .layout = layout,
        .vertex = {
           .nextInChain = nullptr,
           .module = shaders,
           .entryPoint = {
               .data = "vs",
               .length = 2
           },
           .constantCount = 0,
           .constants = nullptr,
           .bufferCount = 1,
           .buffers = &vtxLayout,
        },
        .primitive = {
            .nextInChain = nullptr,
            .topology = WGPUPrimitiveTopology_TriangleList,
            .stripIndexFormat = WGPUIndexFormat_Undefined,
            .frontFace = WGPUFrontFace_CW,
            .cullMode = WGPUCullMode_Back,
            .unclippedDepth = false
        },
        .depthStencil = &depthStencilState,
        .multisample = {
            .nextInChain = nullptr,
            .count = 1,
            .mask = 0xFFFFFFFF,
            .alphaToCoverageEnabled = true
        },
        .fragment = &fragmentState,
    };

    auto pipeline = wgpuDeviceCreateRenderPipeline(device, &desc);

    auto built = std::make_shared<RenderPipeline>(device, pipeline, std::move(bindGroupLayouts), universalBindGroup, immediateDataBytes);
    pipelines.insert_or_assign(name, built);

    return built;
}

glengine::pipeline::wgpu::RenderBundle glengine::pipeline::wgpu::WGPURenderer::BeginRendering(RenderUniforms& uniforms) {
    // upload uniforms for this frame and submit any in-flight writes
    wgpuQueueWriteBuffer(queue, renderUniformsBuffer, 0, &uniforms, sizeof(uniforms));
    wgpuQueueSubmit(queue, 0, nullptr);

    // get surface texture
    CONFIGURE:
    WGPUSurfaceTexture texture;
    WGPUTextureView textureView;
    wgpuSurfaceGetCurrentTexture(surface, &texture);
    switch (texture.status) {
        case WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal:
            textureView = wgpuTextureCreateView(texture.texture, nullptr);
            break;
        case WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal:
        case WGPUSurfaceGetCurrentTextureStatus_Outdated:
            wgpuSurfaceConfigure(surface, &surfConfig);
            goto CONFIGURE;
        case WGPUSurfaceGetCurrentTextureStatus_Timeout:
            // happens on macos when window is obscured. We won't bother rendering then
        default:
            return {};
    }

    // clear textures
    auto encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);

    WGPURenderPassDescriptor descriptor = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    auto attachment = WGPURenderPassDepthStencilAttachment {
        .nextInChain = nullptr,
        .view = depthTextureView,
        .depthLoadOp = WGPULoadOp_Clear,
        .depthStoreOp = WGPUStoreOp_Store,
        .depthClearValue = 1.0,
        .depthReadOnly = false,
        .stencilLoadOp = WGPULoadOp_Clear,
        .stencilStoreOp = WGPUStoreOp_Store,
        .stencilClearValue = 0,
        .stencilReadOnly = false
    };
    auto colorAttachment = WGPURenderPassColorAttachment {
        .nextInChain = nullptr,
        .view = textureView,
        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
        .resolveTarget = nullptr,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = WGPUColor(0, 0, 0, 1)
    };
    descriptor.depthStencilAttachment = &attachment;
    descriptor.colorAttachmentCount = 1;
    descriptor.colorAttachments = &colorAttachment;

    auto pass = wgpuCommandEncoderBeginRenderPass(encoder, &descriptor);
    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);

    return {
        .encoder = encoder,
        .targetTexture = textureView,
        .depthTexture = depthTextureView,
        .surfaceTexture = texture.texture,
        .valid = true
    };
}

void glengine::pipeline::wgpu::WGPURenderer::FinishRendering(RenderBundle bundle) {
    auto command = wgpuCommandEncoderFinish(bundle.encoder, nullptr);

    if (lastFrame != 0) {
        wgpuDevicePoll(device, true, &lastFrame);
    }
    lastFrame = wgpuQueueSubmitForIndex(queue, 1, &command);
    wgpuCommandBufferRelease(command);
    wgpuCommandEncoderRelease(bundle.encoder);

    wgpuTextureViewRelease(bundle.targetTexture);
    wgpuSurfacePresent(surface);
    wgpuTextureRelease(bundle.surfaceTexture);
}

void glengine::pipeline::wgpu::WGPURenderer::Resize(int2 size) {
    surfConfig.width = size.x;
    surfConfig.height = size.y;

    wgpuSurfaceConfigure(surface, &surfConfig);

    if (depthTexture != nullptr) {
        wgpuTextureRelease(depthTexture);
        wgpuTextureViewRelease(depthTextureView);
    }

    auto desc = WGPUTextureDescriptor {
        .nextInChain = nullptr,
        .label = {
           .data = "Main Depth Target",
           .length = WGPU_STRLEN
        },
        .usage = WGPUTextureUsage_RenderAttachment,
        .dimension = WGPUTextureDimension_2D,
        .size = {
           .width = surfConfig.width,
           .height = surfConfig.height,
           .depthOrArrayLayers = 1
        },
        .format = WGPUTextureFormat_Depth24Plus,
        .mipLevelCount = 1,
        .sampleCount = 1,
        .viewFormatCount = 0,
        .viewFormats = nullptr
    };

    depthTexture = wgpuDeviceCreateTexture(device, &desc);
    depthTextureView = wgpuTextureCreateView(depthTexture, nullptr);
}

#include "Shaders.h"

void glengine::pipeline::wgpu::WGPURenderer::buildBuiltinPipelines() {
    auto basicLitShaders = CompileShader(embed_BasicLit_wgsl);
    BuildRenderPipeline("BasicLit", basicLitShaders, {} , sizeof(mat4));

    WGPUBindGroupLayoutEntry basicLitInstancedBindGroupEntry = {
        .nextInChain = nullptr,
        .binding = 0,
        .visibility = WGPUShaderStage_Vertex,
        .bindingArraySize = 0,
        .buffer = {
            .nextInChain = nullptr,
            .type = WGPUBufferBindingType_ReadOnlyStorage,
            .hasDynamicOffset = false,
            .minBindingSize = 0
        },
        .sampler = {},
        .texture = {},
        .storageTexture = {}
    };
    WGPUBindGroupLayoutDescriptor basicLitInstancedBindGroup = {
        .nextInChain = nullptr,
        .label = {},
        .entryCount = 1,
        .entries = &basicLitInstancedBindGroupEntry
    };
    auto basicLitInstancedShaders = CompileShader(embed_BasicLitInstanced_wgsl);
    BuildRenderPipeline("BasicLitInstanced", basicLitInstancedShaders, std::span(&basicLitInstancedBindGroup, 1), 0);
}
