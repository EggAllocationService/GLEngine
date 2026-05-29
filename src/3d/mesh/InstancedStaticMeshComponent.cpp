//
// Created by Kyle Smith on 2026-05-29.
//

#include "3d/mesh/InstancedStaticMeshComponent.h"

#include <cassert>

#include "Engine.h"

glengine::world::mesh::InstancedStaticMeshComponent::InstancedStaticMeshComponent() {
    tracker_ = GetEngine()->GetRenderObjectsManager()->GetObject<objects::InstancedDrawTracker>();
    assert(tracker_ != nullptr);
}

void glengine::world::mesh::InstancedStaticMeshComponent::SetMesh(std::shared_ptr<StaticMesh> mesh) {
    mesh_ = std::move(mesh);
}

void glengine::world::mesh::InstancedStaticMeshComponent::Update(double deltaTime) {
    auto instanceData = objects::instanced::InstanceData {
        .matrix = GetActor()->GetTransformMatrix() * GetTransformMatrix()
    };
    tracker_->Draw(mesh_->mesh, instanceData);
}
