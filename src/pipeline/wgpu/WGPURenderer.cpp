//
// Created by Kyle Smith on 2026-05-28.
//

#include "pipeline/wgpu/WGPURenderer.h"

#include <cassert>
#include <cstring>
#include <atomic>
#include <bit>

#include "glfw3webgpu.h"

#ifdef GLENGINE_TEXT_RENDERING
#include "3d/text/Font.h"
#include "TextRenderingResources.h"
#endif

static void handle_request_adapter(WGPURequestAdapterStatus status,
    WGPUAdapter adapter, WGPUStringView message,
    void* userdata1, void* userdata2) {
    *(WGPUAdapter*)userdata1 = adapter;
}
static void handle_request_device(WGPURequestDeviceStatus status,
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

    auto features = new WGPUNativeFeature[2] { WGPUNativeFeature_Immediates, WGPUNativeFeature_PolygonModeLine };
    WGPUDeviceDescriptor deviceDescriptor = WGPU_DEVICE_DESCRIPTOR_INIT;
    deviceDescriptor.requiredLimits = &requiredLimits;
    deviceDescriptor.requiredFeatures = reinterpret_cast<WGPUFeatureName*>(&features[0]);
    deviceDescriptor.requiredFeatureCount = 2;

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
    auto surfExtras = new WGPUSurfaceConfigurationExtras {
        .chain = {
            .next = nullptr,
            .sType = std::bit_cast<WGPUSType>(WGPUSType_SurfaceConfigurationExtras),
        },
        .desiredMaximumFrameLatency = 3
    };

    surfConfig.nextInChain = &surfExtras->chain;
    surfConfig.format = caps.formats[0]; // set preferred format
    surfConfig.usage = WGPUTextureUsage_RenderAttachment;
    surfConfig.presentMode = WGPUPresentMode_Fifo;
    surfConfig.alphaMode = WGPUCompositeAlphaMode_Auto;
    surfConfig.device = device;

    // create universal bind group layout descriptor
    WGPUBindGroupLayoutEntry *universalEntryDescs = new WGPUBindGroupLayoutEntry[2] {WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
    universalEntryDescs[0].buffer = WGPUBufferBindingLayout { // render uniforms
        .nextInChain = nullptr,
        .type = WGPUBufferBindingType_Uniform,
        .hasDynamicOffset = false,
        .minBindingSize = sizeof(RenderUniforms)
    };
    universalEntryDescs[0].visibility = WGPUShaderStage_Fragment | WGPUShaderStage_Vertex,

    universalEntryDescs[1].binding = 1;
    universalEntryDescs[1].buffer = WGPUBufferBindingLayout { // lighting info
        .nextInChain = nullptr,
        .type = WGPUBufferBindingType_ReadOnlyStorage,
        .hasDynamicOffset = false,
        .minBindingSize = 0
    };
    universalEntryDescs[1].visibility = WGPUShaderStage_Fragment;

    auto universalLayoutDescriptor = WGPUBindGroupLayoutDescriptor {
        .nextInChain = nullptr,
        .label = {
           .data = "Universal Bind Group Layout",
           .length = WGPU_STRLEN
        },
        .entryCount = 2,
        .entries = universalEntryDescs,
    };
    universalBindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &universalLayoutDescriptor);
    delete[] universalEntryDescs;

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
    universalEntries.push_back(entry0);

    universalBindGroup = nullptr;

    int2 size;
    glfwGetWindowSize(window, &size.x, &size.y);
    Resize(size);

    buildBuiltinPipelines();

    transferManager = new TransferManager(this);
    universalDirty = true;
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

    return std::make_shared<GPUMesh>(buffer, nullptr, vertices.size(), sizeof(Vertex), 0, meshIdTracker++);
}

std::shared_ptr<glengine::pipeline::wgpu::RenderPipeline> glengine::pipeline::wgpu::WGPURenderer::
GetRenderPipelineByName(const std::string &name) {
    if (pipelines.contains(name)) {
        return pipelines[name];
    }

    return nullptr;
}

std::shared_ptr<glengine::pipeline::wgpu::RenderPipeline> glengine::pipeline::wgpu::WGPURenderer::BuildRenderPipeline(std::string name,
    WGPUShaderModule shaders, WGPUVertexBufferLayout* vertexLayout, std::span<WGPUBindGroupLayoutDescriptor>
    bindGroups, int immediateDataBytes, RenderPipelineExtras* extras) {

    // manually build pipeline layout
    std::vector<WGPUBindGroupLayout> bindGroupLayouts(bindGroups.size() + 1);

    bindGroupLayouts[0] = universalBindGroupLayout;
    for (int i = 0; i < bindGroups.size(); i++) {
        bindGroupLayouts[i + 1] = wgpuDeviceCreateBindGroupLayout(device, &bindGroups[i]);
    }

    WGPUPipelineLayoutExtras layoutExtras = {
        .chain = {
            .next = nullptr,
            .sType = static_cast<WGPUSType>(WGPUSType_PipelineLayoutExtras),
        },
        .immediateDataSize = static_cast<uint32_t>(immediateDataBytes),
    };

    auto layoutDesc = WGPUPipelineLayoutDescriptor {
        .nextInChain = &layoutExtras.chain,
        .label = {
           .data = name.data(),
           .length = name.length(),
        },
        .bindGroupLayoutCount = bindGroups.size() + 1,
        .bindGroupLayouts = bindGroupLayouts.data(),
        .immediateSize = static_cast<uint32_t>(immediateDataBytes),
    };

    auto layout = wgpuDeviceCreatePipelineLayout(device, &layoutDesc);


    WGPUBlendState blend = WGPU_BLEND_STATE_INIT;
    blend.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blend.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blend.color.operation = WGPUBlendOperation_Add;
    auto target = WGPUColorTargetState {
        .nextInChain = nullptr,
        .format = surfConfig.format,
        .blend = &blend,
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
            .attributeCount =  3,
            .attributes = &attributes[0]
        };
    auto depthStencilState = WGPUDepthStencilState {
        .nextInChain = nullptr,
        .format = WGPUTextureFormat_Depth24Plus,
        .depthWriteEnabled = WGPUOptionalBool_True,
        .depthCompare = extras != nullptr ? extras->depthMode : WGPUCompareFunction_LessEqual,
        .stencilFront = {},
        .stencilBack = {},
        .stencilReadMask = 0,
        .stencilWriteMask = 0,
        .depthBias = 0,
        .depthBiasSlopeScale = 0,
        .depthBiasClamp = 0
    };

    auto primitiveExtras = WGPUPrimitiveStateExtras {
        .chain = {
           .next = nullptr,
           .sType = std::bit_cast<WGPUSType>(WGPUSType_PrimitiveStateExtras),
        },
        .polygonMode = extras != nullptr ? extras->polygonMode : WGPUPolygonMode_Fill,
        .conservative = false
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
           .buffers = vertexLayout != nullptr ? vertexLayout : &vtxLayout,
        },
        .primitive = {
            .nextInChain = &primitiveExtras.chain,
            .topology = extras != nullptr ? extras->primitiveTopology : WGPUPrimitiveTopology_TriangleList,
            .stripIndexFormat = WGPUIndexFormat_Undefined,
            .frontFace = WGPUFrontFace_CW,
            .cullMode = extras != nullptr ? extras->cullMode : WGPUCullMode_Back,
            .unclippedDepth = false
        },
        .depthStencil = &depthStencilState,
        .multisample = {
            .nextInChain = nullptr,
            .count = 1,
            .mask = 0xFFFFFFFF,
            .alphaToCoverageEnabled = false
        },
        .fragment = &fragmentState,
    };

    auto pipeline = wgpuDeviceCreateRenderPipeline(device, &desc);

    auto built = std::make_shared<RenderPipeline>(device, pipeline, std::move(bindGroupLayouts), nullptr, immediateDataBytes);
    pipelines.insert_or_assign(name, built);

    return built;
}

std::shared_ptr<glengine::pipeline::wgpu::ComputePipeline> glengine::pipeline::wgpu::WGPURenderer::
GetComputePipelineByName(const std::string &name) {
    if (computePipelines.contains(name)) {
        return computePipelines[name];
    }

    return nullptr;
}

std::shared_ptr<glengine::pipeline::wgpu::ComputePipeline> glengine::pipeline::wgpu::WGPURenderer::BuildComputePipeline(
    std::string name, WGPUShaderModule kernel, std::string_view entryPoint,
    std::span<WGPUBindGroupLayoutDescriptor> bindGroups, int immediateDataBytes) {

    std::vector<WGPUBindGroupLayout> bindGroupLayouts(bindGroups.size());

    for (int i = 0; i < bindGroups.size(); i++) {
        bindGroupLayouts[i] = wgpuDeviceCreateBindGroupLayout(device, &bindGroups[i]);
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
        .label = {},
        .bindGroupLayoutCount = bindGroups.size(),
        .bindGroupLayouts = bindGroupLayouts.data(),
        .immediateSize = static_cast<uint32_t>(immediateDataBytes),
    };

    auto desc = WGPUComputePipelineDescriptor {
        .nextInChain = nullptr,
        .label = {
            .data = name.data(),
            .length = name.length(),
        },
        .layout = wgpuDeviceCreatePipelineLayout(device, &layoutDesc),
        .compute = {
            .nextInChain = nullptr,
            .module = kernel,
            .entryPoint = {
               .data = entryPoint.data(),
               .length = entryPoint.length()
            },
            .constantCount = 0,
            .constants = nullptr
        }
    };

    auto pipeline = wgpuDeviceCreateComputePipeline(device, &desc);

    auto built = std::make_shared<ComputePipeline>(device, pipeline, std::move(bindGroupLayouts), immediateDataBytes);
    computePipelines.insert_or_assign(name, built);

    return built;
}

void glengine::pipeline::wgpu::WGPURenderer::SetUniversalBindGroupEntry(WGPUBindGroupEntry entry) {
    if (universalEntries.size() < entry.binding + 1) {
        universalEntries.resize(entry.binding + 1);
    }
    universalEntries[entry.binding] = entry;
    universalDirty = true;
}

void glengine::pipeline::wgpu::WGPURenderer::rebuildUniversalBindGroup() {
    if (universalBindGroup != nullptr) {
        wgpuBindGroupRelease(universalBindGroup);
    }

    WGPUBindGroupDescriptor desc = {
        .nextInChain = nullptr,
        .label = {
            .data = "GLEngine: Universal Group",
            .length = WGPU_STRLEN
        },
        .layout = universalBindGroupLayout,
        .entryCount = universalEntries.size(),
        .entries = universalEntries.data()
    };

    universalBindGroup = wgpuDeviceCreateBindGroup(device, &desc);

    universalDirty = false;
}

glengine::pipeline::wgpu::RenderBundle glengine::pipeline::wgpu::WGPURenderer::BeginRendering(RenderUniforms& uniforms) {
    // upload uniforms for this frame and submit any in-flight writes
    transferManager->Transfer(renderUniformsBuffer, 0, &uniforms, sizeof(uniforms));

    if (universalDirty) {
        rebuildUniversalBindGroup();
    }

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
    wgpuRenderPassEncoderSetBindGroup(pass, 0, universalBindGroup, 0, nullptr);

    return {
        .encoder = encoder,
        .passEncoder = pass,
        .targetTexture = textureView,
        .depthTexture = depthTextureView,
        .surfaceTexture = texture.texture,
        .valid = true
    };
}

void glengine::pipeline::wgpu::WGPURenderer::FinishRendering(RenderBundle bundle) {
    if (!bundle.valid) return;
    wgpuRenderPassEncoderEnd(bundle.passEncoder);
    wgpuRenderPassEncoderRelease(bundle.passEncoder);

    auto command = wgpuCommandEncoderFinish(bundle.encoder, nullptr);

    lastFrame = wgpuQueueSubmitForIndex(queue, 1, &command);
    wgpuCommandBufferRelease(command);
    wgpuCommandEncoderRelease(bundle.encoder);

    wgpuTextureViewRelease(bundle.targetTexture);
    wgpuSurfacePresent(surface);
    wgpuTextureRelease(bundle.surfaceTexture);
}

glengine::pipeline::wgpu::ComputeBundle glengine::pipeline::wgpu::WGPURenderer::BeginComputePass() {
    auto cmd = wgpuDeviceCreateCommandEncoder(device, nullptr);
    return {
        .cmd = cmd,
        .encoder = wgpuCommandEncoderBeginComputePass(cmd, nullptr)
    };
}

void glengine::pipeline::wgpu::WGPURenderer::CommitComputePass(ComputeBundle& bundle) {
    wgpuComputePassEncoderEnd(bundle.encoder);
    wgpuComputePassEncoderRelease(bundle.encoder);

    auto buf = wgpuCommandEncoderFinish(bundle.cmd, nullptr);
    wgpuQueueSubmit(queue, 1, &buf);

    wgpuCommandBufferRelease(buf);
    wgpuCommandEncoderRelease(bundle.cmd);
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

std::shared_ptr<glengine::pipeline::wgpu::GPUTexture> glengine::pipeline::wgpu::WGPURenderer::CreateTexture(std::string_view name,
    WGPUTextureUsage usage, WGPUTextureFormat format, unsigned int width, unsigned int height) {

    auto desc = WGPUTextureDescriptor {
        .nextInChain = nullptr,
        .label = {
            .data = name.data(),
            .length = name.length(),
        },
        .usage = usage,
        .dimension = WGPUTextureDimension_2D,
        .size = {
          .width = width,
          .height = height,
          .depthOrArrayLayers = 1
        },
        .format = format,
        .mipLevelCount = 1,
        .sampleCount = 1,
        .viewFormatCount = 0,
        .viewFormats = nullptr
    };

    return std::make_shared<GPUTexture>(wgpuDeviceCreateTexture(device, &desc), format, width, height);
}

glengine::pipeline::wgpu::WrappedBuffer glengine::pipeline::wgpu::WGPURenderer::CreateRawBuffer(std::string_view name,
                                                                                                WGPUBufferUsage usage, unsigned int size) const {

    auto desc = WGPUBufferDescriptor {
        .nextInChain = nullptr,
        .label = {
            .data = name.data(),
            .length = name.length(),
        },
        .usage = usage,
        .size = size,
        .mappedAtCreation = false
    };
    return {wgpuDeviceCreateBuffer(device, &desc)};
}

#include "Shaders.h"

void glengine::pipeline::wgpu::WGPURenderer::buildBuiltinPipelines() {
    auto basicLitShaders = CompileShader(embed_BasicLit_wgsl);
    WGPUBindGroupLayoutEntry basicLitBindGroupEntry = {
        .nextInChain = nullptr,
        .binding = 0,
        .visibility = WGPUShaderStage_Fragment,
        .bindingArraySize = 0,
        .buffer = {
            .nextInChain = nullptr,
            .type = WGPUBufferBindingType_Uniform,
            .hasDynamicOffset = false,
            .minBindingSize = 0
        },
        .sampler = {},
        .texture = {},
        .storageTexture = {}
    };
    WGPUBindGroupLayoutDescriptor basicLitBindGroup = {
        .nextInChain = nullptr,
        .label = {},
        .entryCount = 1,
        .entries = &basicLitBindGroupEntry
    };

    BuildRenderPipeline("BasicLit", basicLitShaders, {}, std::span(&basicLitBindGroup, 1), sizeof(mat4), nullptr);

    WGPUBindGroupLayoutEntry basicLitInstancedBindGroupEntry = {
        .nextInChain = nullptr,
        .binding = 0,
        .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
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
    BuildRenderPipeline("BasicLitInstanced", basicLitInstancedShaders, {}, std::span(&basicLitInstancedBindGroup, 1), 0, nullptr);

#ifdef GLENGINE_TEXT_RENDERING

    auto slugShaders = CompileShader(embed_slug_wgsl);

    auto *slugEntries = new WGPUBindGroupLayoutEntry[3] {
        {
            .nextInChain = nullptr,
            .binding = 0,
            .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
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
        },
        {
            .nextInChain = nullptr,
            .binding = 1,
            .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
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
        },
        {
            .nextInChain = nullptr,
            .binding = 2,
            .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
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
        }
    };

    WGPUBindGroupLayoutDescriptor slugBindGroup = {
        .nextInChain = nullptr,
        .label = {},
        .entryCount = 3,
        .entries = &slugEntries[0]
    };

    auto slugVtxLayouts = new WGPUVertexAttribute[3] {
        {
            .nextInChain = nullptr,
            .format = WGPUVertexFormat_Float32x4,
            .offset = offsetof(world::font::SlugVertex, pos),
            .shaderLocation = 0
        },
        {
            .nextInChain = nullptr,
            .format = WGPUVertexFormat_Float32x4,
            .offset = offsetof(world::font::SlugVertex, color),
            .shaderLocation = 2
        },
        {
        .nextInChain = nullptr,
        .format = WGPUVertexFormat_Uint32x2,
        .offset = offsetof(world::font::SlugVertex, glyphData),
        .shaderLocation = 1
        }
    };

    WGPUVertexBufferLayout slugLayout = {
        .nextInChain = nullptr,
        .stepMode = WGPUVertexStepMode_Vertex,
        .arrayStride = sizeof(world::font::SlugVertex),
        .attributeCount = 3,
        .attributes = slugVtxLayouts
    };

    auto slugExtras = RenderPipelineExtras {
        .polygonMode = WGPUPolygonMode_Fill,
        .depthMode = WGPUCompareFunction_LessEqual,
        .cullMode = WGPUCullMode_None
    };

    BuildRenderPipeline("BuiltinSlug", slugShaders, &slugLayout, std::span(&slugBindGroup, 1), sizeof(mat4), &slugExtras);

    delete[] slugVtxLayouts;
    delete[] slugEntries;
#endif

    auto gizmoShaders = CompileShader(embed_Gizmo_wgsl);

    auto extras = RenderPipelineExtras {
        .polygonMode = WGPUPolygonMode_Line,
        .depthMode = WGPUCompareFunction_Always,
        .cullMode = WGPUCullMode_Back
    };
    BuildRenderPipeline("BuiltinGizmo", gizmoShaders, nullptr, {}, sizeof(mat4) + sizeof(float4), &extras);
}
