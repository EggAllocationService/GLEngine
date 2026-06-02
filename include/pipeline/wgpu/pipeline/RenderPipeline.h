//
// Created by Kyle Smith on 2026-05-28.
//
#pragma once
#include "../GPUMesh.h"
#include "webgpu/webgpu.h"
#include <span>
#include <memory>
#include "glengine_export.h"
#include "Pipeline.h"

/*
 * Standard pipeline layout:
 * Group 0:
 *  Binding 0:
 *  struct RenderUniforms {
 *      mat4 cameraMatrix;
 *      mat4 viewMatrix;
 *      int lightCount;
 *  }
 *  Binding 1:
 *  struct LightInfo {
 *
 *  }[]
 */

namespace glengine::pipeline::wgpu {
    struct RenderBundle;

    class GLENGINE_EXPORT RenderPipeline : public Pipeline {
    public:
        RenderPipeline(WGPUDevice device, WGPURenderPipeline pipeline, std::vector<WGPUBindGroupLayout> layouts, WGPUBindGroup universalGroup, uint32_t immediateDataSize);
        explicit RenderPipeline(RenderPipeline &other);
        ~RenderPipeline();
        void DrawMesh(const RenderBundle& bundle, const GPUMesh& mesh, const void* immediateData);
        void DrawMeshInstanced(const RenderBundle& bundle, const GPUMesh& mesh, int instanceCount);

        void DrawMeshInstancedIndirect(const RenderBundle& bundle, const GPUMesh& mesh, WGPUBuffer indirectBuffer);

        std::shared_ptr<RenderPipeline> CreateInstance();
    private:
        WGPURenderPassEncoder createPass(const RenderBundle& bundle);
        WGPURenderPipeline _pipeline;
        uint32_t _immediateDataSize;
    };
}