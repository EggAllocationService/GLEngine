//
// Created by Kyle Smith on 2025-10-21.
//
#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <fstream>

#include "glengine_export.h"
#include "Vectors.h"

namespace glengine::world::mesh {
    struct GLENGINE_EXPORT PackedVertexData {
        float3 position;
        float3 normal;
        float2 texCoord;
    };
    /// A StaticMesh is a non-animated mesh
    class GLENGINE_EXPORT StaticMesh {
    public:
        static std::unique_ptr<StaticMesh> FromOBJ(std::ifstream& file);

        /// Normalizes the mesh so all vertices are within (-1, -1, -1) to (1, 1, 1)
        void Normalize();

        void Render() const;
    private:
        std::vector<PackedVertexData> vertices_;
        std::vector<int3> faces_;

        bool hasNormals_ = false;
        bool hasTexCoords_ = false;
    };
}
