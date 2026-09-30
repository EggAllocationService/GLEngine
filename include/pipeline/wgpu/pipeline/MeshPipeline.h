//
// Created by Kyle Smith on 2026-05-28.
//
#pragma once
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
    class GLENGINE_EXPORT MeshPipeline : public Pipeline {
    public:
        MeshPipeline(WGPUDevice device, WGPURenderPipeline pipeline, std::vector<WGPUBindGroupLayout> layouts, WGPUBindGroup universalGroup, uint32_t immediateDataSize);
        explicit MeshPipeline(MeshPipeline &other);
        ~MeshPipeline();
        void DispatchMeshTasks(const RenderBundle& bundle, uint32_t x, uint32_t y, uint32_t z, const void* immediateData);
        void DispatchMeshTasksIndirect(const RenderBundle& bundle, WGPUBuffer indirectBuffer, uint64_t offset, const void *immediateData);

        std::shared_ptr<MeshPipeline> CreateInstance();
    private:
        WGPURenderPassEncoder createPass(const RenderBundle& bundle);
        WGPURenderPipeline _pipeline;
        uint32_t _immediateDataSize;
    };
}