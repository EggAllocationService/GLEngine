#pragma once
#include "glengine_export.h"
#include <istream>
#include "Resource.h"
#include "pipeline/wgpu/WGPURenderer.h"


namespace glengine::world::mesh {
    class GLENGINE_EXPORT OptimizedMesh : public Resource {
    public:
        OptimizedMesh(std::istream& file, pipeline::wgpu::WGPURenderer* renderer);
        ~OptimizedMesh() override;
        [[nodiscard]] int GetId() const;

        [[nodiscard]] const pipeline::wgpu::WrappedBuffer& GetMeshlets() const {
            return meshlets;
        }

        [[nodiscard]] const pipeline::wgpu::WrappedBuffer& GetVertices() const {
            return vertices;
        }

        [[nodiscard]] const pipeline::wgpu::WrappedBuffer& GetIndices() const {
            return indices;
        }

        [[nodiscard]] unsigned int GetMeshletCount() const {
            return meshletCount;
        }
    private:
        static int LastId;
        pipeline::wgpu::WrappedBuffer meshlets;
        pipeline::wgpu::WrappedBuffer vertices;
        pipeline::wgpu::WrappedBuffer indices;
        WGPUBindGroup group;
        unsigned int meshletCount;
        int id;
    };
}