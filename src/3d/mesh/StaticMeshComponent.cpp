//
// Created by Kyle Smith on 2025-10-21.
//

#include "3d/mesh/StaticMeshComponent.h"

using namespace glengine::world::mesh;

void StaticMeshComponent::Render() {
    if (mesh_ == nullptr) return;

    mesh_->Render();
}

void StaticMeshComponent::SetMesh(StaticMesh *mesh) {
    mesh_ = std::unique_ptr<StaticMesh>(mesh);
}
