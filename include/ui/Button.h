//
// Created by Kyle Smith on 2026-10-24.
//
#pragma once

#include <functional>
#include <string>

#include "ui/UIElement.h"
#include "glengine_export.h"

namespace glengine::ui {
    class GLENGINE_EXPORT Button : public View {
    public:
        Button();
        explicit Button(std::string label);

        void SetLabel(std::string label);
        void SetFont(std::shared_ptr<world::font::Font> font);

        using ClickHandler = std::function<void()>;
        void AddClickHandler(ClickHandler handler) { clickHandlers_.push_back(std::move(handler)); }

        void Update(double deltaTime) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;
        void OnMouseDown(int button, float2 localPos) override;
        void OnMouseUp(int button, float2 localPos) override;

        float4 Arrange(float4 available) override;
        void CollectRenderCommands(std::vector<UIRenderCommand>& commands, const mat3& parentTransform) override;

    private:
        void UpdateAppearance();
        void EnsureLabel();

        std::string label_;
        std::shared_ptr<world::font::Font> font_;
        std::shared_ptr<class TextBlock> labelBlock_;
        std::vector<ClickHandler> clickHandlers_;
        bool hovered_ = false;
        bool pressed_ = false;

        float4 normalColor_ = float4(0.2f, 0.22f, 0.3f, 1.0f);
        float4 hoverColor_ = float4(0.3f, 0.34f, 0.45f, 1.0f);
        float4 pressedColor_ = float4(0.12f, 0.14f, 0.2f, 1.0f);
    };
}
