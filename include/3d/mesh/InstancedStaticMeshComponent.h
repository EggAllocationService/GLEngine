// Created by Kyle Smith on 2026-05-29.
//
#pragma once
#include "glengine_export.h"
#include "StaticMesh.h"
#include "3d/ActorSceneComponent.h"
#include "3d/texture/StaticTexture2D.h"
#include "3d/objects/InstancedDrawTracker.h"

namespace glengine::world::mesh {
    class GLENGINE_EXPORT InstancedStaticMeshComponent : public ActorPrimitiveComponent {
    public:
        InstancedStaticMeshComponent();

        void SetMesh(std::shared_ptr<StaticMesh>);

        void Update(double deltaTime) override;

        Material material;
    private:
        std::shared_ptr<StaticMesh> mesh_;
        std::shared_ptr<objects::InstancedDrawTracker> tracker_;
    };
}
