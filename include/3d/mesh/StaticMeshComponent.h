//
// Created by Kyle Smith on 2025-10-21.
//
#pragma once
#include "glengine_export.h"
#include "StaticMesh.h"
#include "3d/ActorSceneComponent.h"
#include "engine_GLUT.h"

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

    class GLENGINE_EXPORT StaticMeshComponent : public ActorSceneComponent {
    public:
        void Render() override;

        void SetMesh(std::shared_ptr<StaticMesh>);

        Material material;
    private:
        std::shared_ptr<StaticMesh> mesh_;
    };
}
