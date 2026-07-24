//
// Created by Kyle Smith on 2026-07-24.
//
#pragma once
#include "3d/ActorSceneComponent.h"
#include "glengine_export.h"

namespace glengine::world::components {
    class GLENGINE_EXPORT AxesComponent : public ActorSceneComponent {
    public:
        AxesComponent();
        void Render(const pipeline::wgpu::RenderBundle &, MatrixStack &stack) override;
    private:
        std::shared_ptr<pipeline::wgpu::GPUMesh> mesh;
        std::shared_ptr<pipeline::wgpu::RenderPipeline> pipeline;
    };
}

