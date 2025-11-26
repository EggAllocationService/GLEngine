//
// Created by Kyle Smith on 2025-10-21.
//
#pragma once

#include <fstream>
#include <vector>
#include "engine_GLUT.h"

#include "glengine_export.h"
#include "Resource.h"
#include "Vectors.h"

namespace glengine::world::mesh {
    struct GLENGINE_EXPORT Material {
        float4 Diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
        float4 Specular = float4(0, 0, 0, 1.0f);
        float4 Emissive = float4(0, 0, 0, 1);
        float Shininess = 0.0f;

        /// Sets GL material properties for front and back according to current values
        void Load() const {
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, reinterpret_cast<const float *>(&Diffuse));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, reinterpret_cast<const float *>(&Specular));
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, reinterpret_cast<const float *>(&Emissive));
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, Shininess);
        }
    };

    struct GLENGINE_EXPORT PackedVertexData {
        float3 position;
        float3 normal;
        float2 texCoord;
    };
    /// A StaticMesh is a non-animated mesh
    class GLENGINE_EXPORT StaticMesh : public Resource {
    public:
        void LoadFromFile(std::ifstream& file) override;

        std::vector<PackedVertexData> vertices_;
        std::vector<int3> faces_;
        bool hasTexCoords_ = false;

    private:
        /// Recalculates all normal vectors for the mesh
        void RecalculateNormals();

        /// Normalizes the mesh so all vertices are within (-1, -1, -1) to (1, 1, 1)
        /// This is to ensure meshes aren't gigantic or tiny for no reason
        void normalize();

        bool hasNormals_ = false;
    };
}
