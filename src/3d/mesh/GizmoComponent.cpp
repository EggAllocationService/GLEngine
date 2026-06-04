//
// Created by Kyle Smith on 2026-06-04.
//

#include "../../../include/3d/mesh/GizmoComponent.h"

#include "Engine.h"

struct GizmoParams {
    mat4 matrix;
    float4 color;
};

namespace glengine::world::mesh {
    GizmoComponent::GizmoComponent(const std::shared_ptr<StaticMesh>& mesh) {
        this->mesh = mesh->mesh;
        pipeline = GetEngine()->GetRenderer()->GetRenderPipelineByName("BuiltinGizmo");

        Color = float4(1, 1, 0, 1);
    }

    GizmoComponent::GizmoComponent(std::string_view modelName) {
        this->mesh = GetEngine()->GetResourceManager()->GetResource<StaticMesh>(modelName)->mesh;
        pipeline = GetEngine()->GetRenderer()->GetRenderPipelineByName("BuiltinGizmo");
        Color = float4(1, 1, 0, 1);
    }

    void GizmoComponent::Render(const pipeline::wgpu::RenderBundle &bundle, MatrixStack &stack) {
        GizmoParams params = {
            .matrix = stack,
            .color = Color
        };

        pipeline->DrawMesh(bundle, *mesh, &params);
    }
}
