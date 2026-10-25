//
// Created by Kyle Smith on 2026-10-24.
//

#include "ui/FlexView.h"

#include <algorithm>

namespace glengine::ui {
    FlexView::FlexView(Alignment horizontalAlignment, Alignment verticalAlignment)
        : horizontalAlignment_(horizontalAlignment), verticalAlignment_(verticalAlignment) {}

    float4 FlexView::Arrange(float4 available) {
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

        const bool horizontal = direction_ == Direction::Horizontal;

        // Note: UI uses bottom-left origin, but we follow CSS alignment conventions:
        // Start on the horizontal axis = left, Start on the vertical axis = top.

        // measure children along the main axis
        float totalMain = 0.0f;
        int visibleCount = 0;
        for (const auto& child : children_) {
            if (!child->IsVisible()) continue;
            totalMain += horizontal ? child->size.x : child->size.y;
            visibleCount++;
        }
        if (visibleCount > 1) {
            totalMain += gap_ * (visibleCount - 1);
        }

        const float contentMain = horizontal ? content.z : content.w;
        const float contentCross = horizontal ? content.w : content.z;
        const Alignment mainAlignment = horizontal ? horizontalAlignment_ : verticalAlignment_;
        const Alignment crossAlignment = horizontal ? verticalAlignment_ : horizontalAlignment_;

        // starting offset along the main axis, in CSS terms (left-to-right or top-to-bottom)
        float offset;
        switch (mainAlignment) {
            case Alignment::End:
                offset = contentMain - totalMain;
                break;
            case Alignment::Center:
                offset = (contentMain - totalMain) * 0.5f;
                break;
            case Alignment::Start:
            case Alignment::Stretch:
            default:
                offset = 0.0f;
                break;
        }

        for (const auto& child : children_) {
            if (!child->IsVisible()) continue;

            const float main = horizontal ? child->size.x : child->size.y;

            // cross-axis size and CSS-space offset (top-to-bottom for the vertical axis)
            float crossSize;
            switch (crossAlignment) {
                case Alignment::Stretch:
                    crossSize = contentCross;
                    break;
                default:
                    crossSize = horizontal ? child->size.y : child->size.x;
                    break;
            }

            float crossOffset;
            switch (crossAlignment) {
                case Alignment::End:
                    crossOffset = contentCross - crossSize;
                    break;
                case Alignment::Center:
                    crossOffset = (contentCross - crossSize) * 0.5f;
                    break;
                case Alignment::Start:
                case Alignment::Stretch:
                default:
                    crossOffset = 0.0f;
                    break;
            }

            if (horizontal) {
                // CSS left-to-right matches screen left-to-right
                const float x = content.x + offset;
                // CSS top-to-bottom maps to bottom-left origin by flipping within the content area
                const float y = content.y + contentCross - crossOffset - crossSize;
                child->Arrange({x, y, main, crossSize});
            } else {
                const float x = content.x + crossOffset;
                // CSS top-to-bottom: offset is measured from the top of the content area
                const float y = content.y + content.w - offset - main;
                child->Arrange({x, y, crossSize, main});
            }

            offset += main + gap_;
        }

        return finalRect_;
    }
}
