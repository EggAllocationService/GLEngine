//
// Created by Kyle Smith on 2026-06-04.
//
#pragma once
#include "3d/ActorSceneComponent.h"
#include "3d/mesh/StaticMesh.h"

namespace glengine::world::mesh {
    class GizmoComponent : public ActorSceneComponent {
    public:
        GizmoComponent(const std::shared_ptr<StaticMesh>& mesh);
        GizmoComponent(std::string_view modelName);
        void Render(const pipeline::wgpu::RenderBundle &, MatrixStack &stack) override;

        float4 Color;
    private:
        std::shared_ptr<pipeline::wgpu::RenderPipeline> pipeline;
        std::shared_ptr<pipeline::wgpu::GPUMesh> mesh;
    };
}
