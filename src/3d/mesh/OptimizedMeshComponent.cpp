//
// Created by Kyle Smith on 2026-10-01.
//

#include "3d/mesh/OptimizedMeshComponent.h"

#include "Engine.h"

namespace glengine::world::mesh {
    OptimizedMeshComponent::OptimizedMeshComponent() {
        tracker_ = GetEngine()->GetRenderObjectsManager()->GetObject<objects::OptimizedDrawTracker>();
    }

    OptimizedMeshComponent::OptimizedMeshComponent(std::string_view name) : OptimizedMeshComponent() {
        mesh_ = GetEngine()->GetResourceManager()->GetResource<OptimizedMesh>(name);
    }

    void OptimizedMeshComponent::Update(double deltaTime) {
        if (mesh_ != nullptr) {
            auto m = objects::OptimizedMeshInstance {
                .transform = GetActor()->GetTransformMatrix() * GetTransformMatrix()
            };
            tracker_->Draw(mesh_, m);
        }
    }
}
