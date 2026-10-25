//
// Created by Kyle Smith on 2026-10-24.
//

#include "ui/TextBlock.h"

#include "GLMath.h"

namespace glengine::ui {
    TextBlock::TextBlock() {
        // labels should never intercept clicks meant for their parent
        mouseInteraction = MouseInteraction::Ignore;
    }

    TextBlock::TextBlock(std::string text) : text_(std::move(text)) {
        // labels should never intercept clicks meant for their parent
        mouseInteraction = MouseInteraction::Ignore;
    }

    void TextBlock::SetText(std::string text) {
        text_ = std::move(text);
        mesh_.reset();
    }

    void TextBlock::RebuildMesh() {
        if (!font_ || text_.empty()) {
            mesh_.reset();
            textSize_ = float2(0, 0);
            return;
        }

        mesh_ = font_->PrepareText(text_.c_str(), &textSize_);
    }

    float4 TextBlock::Arrange(float4 available) {
        RebuildMesh();

        float pixelScale = 1.0f;
        if (textSize_.y > 0.0f) {
            pixelScale = fontSize_ / textSize_.y;
        }

        const float2 pixelSize = textSize_ * pixelScale;
        scale_ = pixelScale;

        finalRect_ = {
            available.x + position.x,
            available.y + position.y,
            pixelSize.x,
            pixelSize.y
        };
        return finalRect_;
    }

    void TextBlock::CollectRenderCommands(std::vector<UIRenderCommand>& commands, const mat3& parentTransform) {
        if (!visible_ || !mesh_ || !font_) {
            return;
        }

        const auto local = math::translate2D(float2(finalRect_.x + finalRect_.z * 0.5f, finalRect_.y + finalRect_.w * 0.5f)) *
                           math::scale2D(float2(scale_, scale_));

        UIRenderCommand command;
        command.type = UIRenderCommand::Type::Text;
        command.zIndex = zIndex;
        command.transform = parentTransform * local;
        command.size = float2(finalRect_.z, finalRect_.w);
        command.color = foregroundColor;
        command.textMesh = mesh_;
        command.font = font_;
        commands.push_back(command);
    }
}
