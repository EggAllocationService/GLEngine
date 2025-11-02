//
// Created by Kyle Smith on 2025-10-21.
//

#include "3d/mesh/StaticMeshComponent.h"

using namespace glengine::world::mesh;

void StaticMeshComponent::Render() {
    if (mesh_ == nullptr) return;

    material.Load();

    mesh_->Render();
}

void StaticMeshComponent::SetMesh(std::shared_ptr<StaticMesh> mesh) {
    mesh_ = std::move(mesh);
}
