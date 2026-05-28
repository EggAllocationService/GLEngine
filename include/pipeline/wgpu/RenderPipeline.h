//
// Created by Kyle Smith on 2026-05-28.
//
#pragma once
#include "GPUMesh.h"
#include "webgpu/webgpu.h"
#include <span>

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

    class RenderPipeline {
    public:
        RenderPipeline(WGPUDevice device, WGPURenderPipeline pipeline, std::vector<WGPUBindGroupLayout> layouts, WGPUBindGroup universalGroup, uint32_t immediateDataSize);
        void DrawMesh(const RenderBundle& bundle, const GPUMesh& mesh, const void* immediateData);
        void DrawMeshInstanced(const RenderBundle& bundle, const GPUMesh& mesh, int instanceCount);
        void SetBinding(int group, WGPUBindGroupEntry entry);
        void CommitBindings();
    private:
        WGPURenderPassEncoder createPass(const RenderBundle& bundle);
        std::vector<WGPUBindGroup> _groups;
        std::vector<std::vector<WGPUBindGroupEntry>> _entries;
        std::vector<bool> _dirty;
        std::vector<WGPUBindGroupLayout> _layouts;
        WGPURenderPipeline _pipeline;
        WGPUDevice _device;
        uint32_t _immediateDataSize;
    };
}