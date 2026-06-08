//
// Created by Kyle Smith on 2026-06-04.
//
#pragma once
#include "3d/ActorSceneComponent.h"
#include "3d/mesh/StaticMesh.h"

namespace glengine::world::mesh {
    class GizmoComponent : public ActorSceneComponent {
    public:
        GLENGINE_EXPORT GizmoComponent(const std::shared_ptr<StaticMesh>& mesh);
        GLENGINE_EXPORT GizmoComponent(std::string_view modelName);
        GLENGINE_EXPORT void Render(const pipeline::wgpu::RenderBundle &, MatrixStack &stack) override;

        float4 Color;
    private:
        std::shared_ptr<pipeline::wgpu::RenderPipeline> pipeline;
        std::shared_ptr<pipeline::wgpu::GPUMesh> mesh;
    };
}
