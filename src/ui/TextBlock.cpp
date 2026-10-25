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
            builtExpandEm_ = -1.0f;
            return;
        }

        // expand glyph quads by one pixel (converted to mesh units) so the
        // Slug coverage math can antialias past the outline. The expansion
        // depends on the render scale, so rebuild when it changes materially.
        const float expandEm = builtExpandEm_ < 0.0f ? 0.0f : 1.0f / scale_;
        if (mesh_ && builtExpandEm_ > 0.0f &&
            fabsf(expandEm - builtExpandEm_) < builtExpandEm_ * 0.5f) {
            return;
        }

        mesh_ = font_->PrepareText(text_.c_str(), &textSize_, expandEm, mesh_);
        builtExpandEm_ = expandEm;
    }

    float4 TextBlock::Arrange(float4 available) {
        RebuildMesh();

        // fontSize_ is pixels per em; glyph size stays constant regardless of
        // the string's content, only the layout rect grows/shrinks
        const float2 pixelSize = textSize_ * fontSize_;
        scale_ = fontSize_;

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
