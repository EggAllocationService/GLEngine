//
// Created by Kyle Smith on 2025-10-21.
//

#include "3d/mesh/StaticMeshComponent.h"

using namespace glengine::world::mesh;

void StaticMeshComponent::Render() {
    if (mesh_ == nullptr) return;

    material.Load();
    if (texture_ != nullptr) {
        glEnable(GL_TEXTURE_2D);
        texture_->Bind();
    }

    mesh_->Render();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void StaticMeshComponent::SetMesh(std::shared_ptr<StaticMesh> mesh) {
    mesh_ = std::move(mesh);
}

void StaticMeshComponent::SetTexture(std::shared_ptr<texture::StaticTexture2D> texture) {
    texture_ = std::move(texture);
}
