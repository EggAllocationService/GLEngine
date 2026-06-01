//
// Created by Kyle Smith on 2026-05-31.
//
#pragma once
#include "Font.h"
#include "3d/ActorSceneComponent.h"

namespace glengine::world::font {
    class SlugTextComponent : public ActorSceneComponent {
    public:
        SlugTextComponent();
        ~SlugTextComponent();
        void Render(const pipeline::wgpu::RenderBundle &, MatrixStack &stack) override;

    private:
        Font *font;
        std::shared_ptr<pipeline::wgpu::GPUMesh> mesh;
    };
}
