//
// Created by Kyle Smith on 2026-05-28.
//
#pragma once

#include "webgpu/webgpu.h"
#include "glengine_export.h"
#include "Resource.h"

/*
 * Standard vertex layout:
 * float4 position
 * float4 normal
 * float2 uv
 */

namespace glengine::pipeline::wgpu {
    class GLENGINE_EXPORT GPUMesh : public glengine::Resource {
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

        /// Buffer capacities in elements, used to decide if the mesh can be
        /// rewritten in place instead of allocating new buffers.
        [[nodiscard]] int GetVertexCapacity() const {
            return vertexCapacity;
        }
        [[nodiscard]] int GetIndexCapacity() const {
            return indexCapacity;
        }

        /// Grows the buffer capacities (or leaves them unchanged if smaller).
        void SetBufferCapacities(int vertices, int indices);
        /// Updates the element counts after an in-place rewrite.
        void SetDrawCounts(int vertices, int indices);

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
        int vertexCapacity = 0;
        int indexCapacity = 0;
        int id;
        int vertexStride;
    };
}
