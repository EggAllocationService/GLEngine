//
// Created by Kyle Smith on 2026-08-11.
//

#ifndef GLENGINE_POSTPROCESSEFFECT_H
#define GLENGINE_POSTPROCESSEFFECT_H
#include "webgpu/wgpu.h"

namespace glengine::pipeline::wgpu::post {

    /**
     * Post processing effects have a much more limited pipeline layout
     * Bind group 0 is the same as render pipelines; global uniforms and lighting info
     * Bind group 1 contains the scene color and depth textures, and a sampler
     * The only data that can be passed is immediate, up to 128 bytes.
     */
    class PostProcessEffect {
    public:
        PostProcessEffect(WGPURenderPipeline pipeline, unsigned int immediateDataSize) {
            this->pipeline = pipeline;
            this->immediateDataSize = immediateDataSize;
        }

        operator WGPURenderPipeline () const {
            return pipeline;
        }

        unsigned int GetImmediateDataSize() const {
            return immediateDataSize;
        }
    private:
        WGPURenderPipeline pipeline;
        unsigned int immediateDataSize;
    };
}

#endif //GLENGINE_POSTPROCESSEFFECT_H
