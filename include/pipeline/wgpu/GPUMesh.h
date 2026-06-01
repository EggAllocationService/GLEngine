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
        GPUMesh(WGPUBuffer vertices, WGPUBuffer indices, int vertexCount, int vertexStride, int indexCount, int id);
        ~GPUMesh();
        [[nodiscard]] WGPUBuffer GetVertices() const;
        [[nodiscard]] WGPUBuffer GetIndices() const;
        [[nodiscard]] int GetVertexCount() const {
            return vertexCount;
        }
        [[nodiscard]] int GetVertexStride() const {
            return vertexStride;
        }
        [[nodiscard]] int GetIndexCount() const {
            return indexCount;
        }

        [[nodiscard]] int GetId() const {
            return id;
        }

        [[nodiscard]] bool IsIndexed() const {
            return indexCount != 0;
        }
    private:
        WGPUBuffer vertices;
        WGPUBuffer indices;
        int vertexCount;
        int indexCount;
        int id;
        int vertexStride;
    };
}
