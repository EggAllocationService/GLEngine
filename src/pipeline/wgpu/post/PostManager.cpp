//
// Created by Kyle Smith on 2026-08-11.
//

#include "pipeline/wgpu/post/PostManager.h"
#include "pipeline/wgpu/WGPURenderer.h"
#include "Shaders.h"

namespace glengine::pipeline::wgpu::post {
    PostManager::PostManager(WGPURenderer *renderer, WGPUBindGroupLayout universalLayout, WGPUTextureFormat colorFormat) {
        WGPUBindGroupLayoutEntry sceneEntries[3] = {WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
        sceneEntries[0].texture = {
            .nextInChain = nullptr,
            .sampleType = WGPUTextureSampleType_Float,
            .viewDimension = WGPUTextureViewDimension_2D,
            .multisampled = false
        };
        sceneEntries[1].texture = {
            .nextInChain = nullptr,
            .sampleType = WGPUTextureSampleType_Depth,
            .viewDimension = WGPUTextureViewDimension_2D,
            .multisampled = false
        };
        sceneEntries[2].sampler = {
            .nextInChain = nullptr,
            .type = WGPUSamplerBindingType_Filtering
        };

        for (int i = 0; i < 3; i++) {
            sceneEntries[i].binding = i;
            sceneEntries[i].visibility = WGPUShaderStage_Fragment;
        }


        WGPUBindGroupLayoutDescriptor sceneDesc = {
            .nextInChain = nullptr,
            .label = {},
            .entryCount = 3,
            .entries = &sceneEntries[0],
        };

        auto sceneLayout = wgpuDeviceCreateBindGroupLayout(renderer->GetDevice(), &sceneDesc);

        auto samplerDesc = WGPUSamplerDescriptor {
            .nextInChain = nullptr,
            .label = {},
            .addressModeU = WGPUAddressMode_ClampToEdge,
            .addressModeV = WGPUAddressMode_ClampToEdge,
            .addressModeW = WGPUAddressMode_ClampToEdge,
            .magFilter = WGPUFilterMode_Linear,
            .minFilter = WGPUFilterMode_Linear,
            .mipmapFilter = WGPUMipmapFilterMode_Linear,
            .lodMinClamp = 0,
            .lodMaxClamp = 32,
            .compare = WGPUCompareFunction_Undefined,
            .maxAnisotropy = 1
        };

        this->renderer = renderer;
        this->layouts[0] = universalLayout;
        this->layouts[1] = sceneLayout;
        this->sampler = wgpuDeviceCreateSampler(renderer->GetDevice(), &samplerDesc);
        this->vertexShader = renderer->CompileShader(embed_Postvtx_wgsl);

        this->SceneGroups[0] = nullptr;
        this->SceneGroups[1] = nullptr;

        this->colorFormat = colorFormat;
    }

    std::shared_ptr<PostProcessEffect> PostManager::Compile(const char *shader, unsigned int immediateSize) {
        std::string preamble(embed_Postpre_wgsl, embed_Postpre_wgsl_length);
        auto combined = preamble + std::string(shader);

        auto extras = WGPUPipelineLayoutExtras {
            .chain = {
                .next = nullptr,
                .sType = std::bit_cast<WGPUSType>(WGPUSType_PipelineLayoutExtras)
            },
            .immediateDataSize = immediateSize
        };
        auto layoutDesc = WGPUPipelineLayoutDescriptor {
            .nextInChain = &extras.chain,
            .label = {},
            .bindGroupLayoutCount = 2,
            .bindGroupLayouts = &layouts[0],
            .immediateSize = immediateSize
        };
        auto layout = wgpuDeviceCreatePipelineLayout(renderer->GetDevice(), &layoutDesc);

        auto colorState = WGPUColorTargetState {
            .nextInChain = nullptr,
            .format = colorFormat,
            .blend = nullptr,
            .writeMask = WGPUColorWriteMask_All
        };
        auto fragmentState = WGPUFragmentState {
            .nextInChain = nullptr,
            .module = renderer->CompileShader(combined.data()),
            .entryPoint = {
                .data = "post",
                .length = 4
            },
            .constantCount = 0,
            .constants = nullptr,
            .targetCount = 1,
            .targets = &colorState,
        };
        auto pipelineDesc = WGPURenderPipelineDescriptor {
            .nextInChain = nullptr,
            .label = {},
            .layout = layout,
            .vertex = {
                .nextInChain = nullptr,
                .module = vertexShader,
                .entryPoint = {
                    .data = "vs",
                    .length = 2
                },
                .constantCount = 0,
                .constants = nullptr,
                .bufferCount = 0,
                .buffers = nullptr
            },
            .primitive = {
                .nextInChain = nullptr,
                .topology = WGPUPrimitiveTopology_TriangleStrip,
                .stripIndexFormat = WGPUIndexFormat_Undefined,
                .frontFace = WGPUFrontFace_CCW,
                .cullMode = WGPUCullMode_None,
                .unclippedDepth = false
            },
            .depthStencil = nullptr,
            .multisample = WGPU_MULTISAMPLE_STATE_INIT,
            .fragment = &fragmentState,
        };

        auto pipeline = wgpuDeviceCreateRenderPipeline(renderer->GetDevice(), &pipelineDesc);

        return std::make_shared<PostProcessEffect>(pipeline, immediateSize);
    }

    void PostManager::HandleResize(std::shared_ptr<GPUTexture> colorTextures[2],
        std::shared_ptr<GPUTexture> &depthTexture) {

        if (SceneGroups[0] != nullptr) {
            wgpuBindGroupRelease(SceneGroups[0]);
            wgpuBindGroupRelease(SceneGroups[1]);
        }

        WGPUBindGroupEntry entries[3] = {WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT};
        for (int i = 0; i < 3; i++) {
            entries[i].binding = i;
        }

        entries[0].textureView = *colorTextures[0];
        entries[1].textureView = *depthTexture;
        entries[2].sampler = sampler;


        WGPUBindGroupDescriptor desc = {
            .nextInChain = nullptr,
            .label = {},
            .layout = layouts[1],
            .entryCount = 3,
            .entries = &entries[0]
        };

        SceneGroups[0] = wgpuDeviceCreateBindGroup(renderer->GetDevice(), &desc);

        entries[0].textureView = *colorTextures[1];
        SceneGroups[1] = wgpuDeviceCreateBindGroup(renderer->GetDevice(), &desc);
    }
}