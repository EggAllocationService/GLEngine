//
// Created by Kyle Smith on 2026-05-28.
//
#pragma once

#include "webgpu/webgpu.h"

/*
 * Standard vertex layout:
 * float4 position
 * float4 normal
 * float2 uv
 */

namespace glengine::pipeline::wgpu {
    class GPUMesh {
    public:
        GPUMesh(WGPUBuffer buffer, int vertexCount, int id);
        ~GPUMesh();
        [[nodiscard]] WGPUBuffer GetBuffer() const;
        [[nodiscard]] int GetVertexCount() const;

        [[nodiscard]] int GetId() const {
            return id;
        }
    private:
        WGPUBuffer buffer;
        int vertexCount;
        int id;
    };
}
