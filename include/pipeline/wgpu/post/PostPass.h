//
// Created by Kyle Smith on 2026-08-11.
//

#ifndef GLENGINE_POST_H
#define GLENGINE_POST_H
#include <memory>

#include "PostProcessEffect.h"
#include "pipeline/wgpu/GPUTexture.h"

namespace glengine::pipeline::wgpu::post {

    struct GLENGINE_EXPORT PostPass {
        std::shared_ptr<GPUTexture> colorTextures[2];
        std::shared_ptr<GPUTexture> depthTexture;
        WGPUBindGroup sceneBindGroups[2];
        WGPUBindGroup universalBindGroup;
        WGPUCommandEncoder encoder;
        int source;

        void Execute(PostProcessEffect& effect, const void *immediateData);
    };
}
#endif //GLENGINE_POST_H
