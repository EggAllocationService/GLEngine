//
// Created by Kyle Smith on 2026-08-11.
//

#ifndef GLENGINE_POSTMANAGER_H
#define GLENGINE_POSTMANAGER_H
#include "PostProcessEffect.h"
#include "webgpu/wgpu.h"
#include <memory>
#include "glengine_export.h"

namespace glengine::pipeline::wgpu {
    class GPUTexture;
    class WGPURenderer;
}

namespace glengine::pipeline::wgpu::post {
    class GLENGINE_EXPORT PostManager {
    public:
        PostManager(WGPURenderer* renderer, WGPUBindGroupLayout universalLayout);
        std::shared_ptr<PostProcessEffect> Compile(const char* shader, unsigned int immediateSize);
        void HandleResize(std::shared_ptr<GPUTexture> colorTexture[2], std::shared_ptr<GPUTexture>& depthTexture);

        WGPUBindGroup SceneGroups[2];
    private:
        WGPURenderer* renderer;
        WGPUShaderModule vertexShader;
        WGPUBindGroupLayout layouts[2];
        WGPUSampler sampler;

    };
}

#endif //GLENGINE_POSTMANAGER_H
