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

namespace glengine::world::mesh {
    struct GLENGINE_EXPORT PackedVertexData {
        float3 position;
        float3 normal;
        float2 texCoord;
    };
    /// A StaticMesh is a non-animated mesh
    class GLENGINE_EXPORT StaticMesh : public Resource {
    public:
        void LoadFromFile(std::ifstream& file) override;

        void Render() const;

        /// Recalculates all normal vectors for the mesh
        void RecalculateNormals();
    private:

        /// Normalizes the mesh so all vertices are within (-1, -1, -1) to (1, 1, 1)
        /// This is to ensure meshes aren't gigantic or tiny for no reason
        void normalize();

        /// <summary>
        ///  Compiles a command list to accelerate rendering 
        /// </summary>
        void generateCommandList();

        std::vector<PackedVertexData> vertices_;
        std::vector<int3> faces_;

        bool hasNormals_ = false;
        bool hasTexCoords_ = false;

        unsigned int commandList = 0;
    };
}
