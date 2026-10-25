//
// Created by Kyle Smith on 2026-10-24.
//

#include "ui/Button.h"

#include "ui/TextBlock.h"
#include "GLMath.h"

namespace glengine::ui {
    Button::Button() {
        cursor = Cursor::Pointer;
        labelBlock_ = AddElement<TextBlock>();
        labelBlock_->SetFontSize(20.0f);
    }

    Button::Button(std::string label) : Button() {
        SetLabel(std::move(label));
    }

    void Button::SetLabel(std::string label) {
        label_ = std::move(label);
        labelBlock_->SetText(label_);
    }

    void Button::SetFont(std::shared_ptr<world::font::Font> font) {
        font_ = std::move(font);
        labelBlock_->SetFont(font_);
    }

    float4 Button::Arrange(float4 available) {
        finalRect_ = {
            available.x + margin.x,
            available.y + margin.y,
            size.x > 0 ? size.x : std::max(0.0f, available.z - margin.x - margin.z),
            size.y > 0 ? size.y : std::max(0.0f, available.w - margin.y - margin.w)
        };

        if (labelBlock_ && labelBlock_->IsVisible()) {
            labelBlock_->Arrange({0, 0, 0, 0});
            const float2 labelSize = float2(labelBlock_->GetFinalRect().z, labelBlock_->GetFinalRect().w);
            const float2 offset = (float2(finalRect_.z, finalRect_.w) - labelSize) * 0.5f;
            labelBlock_->position = float2(offset.x, offset.y);
            labelBlock_->Arrange(finalRect_);
        }

        return finalRect_;
    }

    void Button::CollectRenderCommands(std::vector<UIRenderCommand>& commands, const mat3& parentTransform) {
        UpdateAppearance();
        View::CollectRenderCommands(commands, parentTransform);
    }

    void Button::UpdateAppearance() {
        if (pressed_) {
            backgroundColor = pressedColor_;
        } else if (hovered_) {
            backgroundColor = hoverColor_;
        } else {
            backgroundColor = normalColor_;
        }
        labelBlock_->foregroundColor = foregroundColor;
    }

    void Button::OnMouseEnter() {
        hovered_ = true;
    }

    void Button::OnMouseLeave() {
        hovered_ = false;
        pressed_ = false;
    }

    void Button::OnMouseDown(int button, float2 localPos) {
        pressed_ = true;
    }

    void Button::OnMouseUp(int button, float2 localPos) {
        if (pressed_) {
            pressed_ = false;
            if (hovered_) {
                for (const auto& handler : clickHandlers_) {
                    handler();
                }
            }
        }
    }
}
