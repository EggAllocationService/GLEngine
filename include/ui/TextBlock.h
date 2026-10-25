//
// Created by Kyle Smith on 2026-10-24.
//
#pragma once

#include <string>

#include "ui/UIElement.h"
#include "3d/text/Font.h"
#include "glengine_export.h"

namespace glengine::ui {
    class GLENGINE_EXPORT TextBlock : public Element {
    public:
        TextBlock();
        explicit TextBlock(std::string text);

        void SetText(std::string text);
        void SetFont(std::shared_ptr<world::font::Font> font) { font_ = std::move(font); RebuildMesh(); }
        void SetFontSize(float size) { fontSize_ = size; }

        float4 Arrange(float4 available) override;
        void CollectRenderCommands(std::vector<UIRenderCommand>& commands, const mat3& parentTransform) override;

    private:
        void RebuildMesh();

        std::string text_;
        std::shared_ptr<world::font::Font> font_;
        std::shared_ptr<pipeline::wgpu::GPUMesh> mesh_;
        float2 textSize_ = float2(0, 0);
        float fontSize_ = 24.0f;
        float scale_ = 1.0f;
        float builtExpandEm_ = -1.0f;
    };
}
