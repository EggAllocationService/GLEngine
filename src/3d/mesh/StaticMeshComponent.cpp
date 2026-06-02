//
// Created by Kyle Smith on 2025-10-21.
//

#include "3d/mesh/StaticMeshComponent.h"

#include "Engine.h"
#include "3d/Actor.h"

using namespace glengine::world::mesh;

StaticMeshComponent::StaticMeshComponent() {
    auto renderer = GetActor()->GetEngine()->GetRenderer();
    pipeline_ = renderer->GetRenderPipelineByName("BasicLit")->CreateInstance();
    material = renderer->AllocateObject<Material>(WGPUBufferUsage_Uniform);

    WGPUBindGroupEntry entry = WGPU_BIND_GROUP_ENTRY_INIT;
    entry.buffer = material;

    pipeline_->SetBinding(1, entry);
    pipeline_->CommitBindings();
}

void StaticMeshComponent::Render(const pipeline::wgpu::RenderBundle& bundle, MatrixStack& stack) {
    if (mesh_ == nullptr) return;
    material.Commit();

    auto matrix = static_cast<mat4>(stack);
    pipeline_->DrawMesh(bundle, *mesh_->mesh, &matrix);
}

void StaticMeshComponent::SetMesh(std::shared_ptr<StaticMesh> mesh) {
    mesh_ = std::move(mesh);
}

void StaticMeshComponent::SetTexture(std::shared_ptr<texture::StaticTexture2D> texture) {
    texture_ = std::move(texture);
}
