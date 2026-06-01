//
// Created by Kyle Smith on 2026-05-31.
//
#pragma once
#include "Font.h"
#include "3d/ActorSceneComponent.h"
#include "glengine_export.h"

namespace glengine::world::font {
    class GLENGINE_EXPORT SlugTextComponent : public ActorSceneComponent {
    public:
        SlugTextComponent();
        void Render(const pipeline::wgpu::RenderBundle &, MatrixStack &stack) override;

        void SetFont(std::shared_ptr<Font> newFont);
        void SetText(std::string_view newText);

    private:
        std::shared_ptr<pipeline::wgpu::GPUMesh> mesh;
        std::shared_ptr<Font> font;
        std::string text;
    };
}
