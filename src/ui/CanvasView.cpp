//
// Created by Kyle Smith on 2026-10-24.
//

#include "ui/CanvasView.h"

#include <algorithm>

namespace glengine::ui {
    CanvasView::CanvasView() = default;

    float4 CanvasView::Arrange(float4 available) {
        finalRect_ = {
            available.x + margin.x,
            available.y + margin.y,
            size.x > 0 ? size.x : std::max(0.0f, available.z - margin.x - margin.z),
            size.y > 0 ? size.y : std::max(0.0f, available.w - margin.y - margin.w)
        };

        const float4 content = {
            finalRect_.x + padding.x,
            finalRect_.y + padding.y,
            std::max(0.0f, finalRect_.z - padding.x - padding.z),
            std::max(0.0f, finalRect_.w - padding.y - padding.w)
        };

        for (const auto& child : children_) {
            if (!child->IsVisible()) continue;

            float childWidth = child->size.x > 0 ? child->size.x : content.z;
            float childHeight = child->size.y > 0 ? child->size.y : content.w;

            child->Arrange({
                content.x + child->position.x,
                content.y + child->position.y,
                childWidth,
                childHeight
            });
        }

        return finalRect_;
    }
}
