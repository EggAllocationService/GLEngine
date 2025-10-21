//
// Created by Kyle Smith on 2025-10-21.
//
#pragma once
#include "glengine_export.h"
#include "StaticMesh.h"
#include "3d/ActorSceneComponent.h"

namespace glengine::world::mesh {
    class GLENGINE_EXPORT StaticMeshComponent : public ActorSceneComponent {
    public:
        void Render() override;

        void SetMesh(StaticMesh *);

    private:
        std::unique_ptr<StaticMesh> mesh_;
    };
}
