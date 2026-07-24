//
// Created by Kyle Smith on 2026-07-24.
//

#include "3d/components/AxesComponent.h"

#include "Engine.h"


namespace glengine::world::components {
    AxesComponent::AxesComponent() {
        mesh = GetEngine()->GetResourceManager()->GetResource<pipeline::wgpu::GPUMesh>("/builtin/models/axes.obj");
        pipeline = GetEngine()->GetRenderer()->GetRenderPipelineByName("BuiltinAxes");
    }

    void AxesComponent::Render(const pipeline::wgpu::RenderBundle &bundle, MatrixStack &stack) {
        mat4 m = stack;
        pipeline->DrawMesh(bundle, *mesh, &m);
    }
}
