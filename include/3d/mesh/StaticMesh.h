//
// Created by Kyle Smith on 2025-10-21.
//
#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <fstream>

#include "glengine_export.h"
#include "Resource.h"
#include "Vectors.h"
#include "engine_GLUT.h"

namespace glengine::world::mesh {
    struct GLENGINE_EXPORT PackedData {
        float3 position;
        float3 normal;
        float2 texCoord;
    };
    /// A StaticMesh is a non-animated mesh
    class GLENGINE_EXPORT StaticMesh : public Resource {
    public:
        /// Loads mesh data from a .obj file
        /// Removes existing data if present
        void LoadFromFile(std::ifstream& file) override;

        void Render() const;
    private:
        /// Normalizes the mesh so all vertices are within (-1, -1, -1) to (1, 1, 1)
        void normalize();

        /// uploads packed vertex data to the GPU
        void uploadToGPU();

        std::vector<PackedData> vertices_;
        std::vector<int3> faces_;

        bool hasNormals_ = false;
        bool hasTexCoords_ = false;

        GLuint vertexBuffer_ = 0;
        int uploadedCount = 0;
    };
}
