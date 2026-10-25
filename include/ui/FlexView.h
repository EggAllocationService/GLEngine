//
// Created by Kyle Smith on 2026-10-24.
//
#pragma once

#include "ui/UIElement.h"
#include "glengine_export.h"

namespace glengine::ui {
    /// Arranges children along a single axis with optional alignment and padding.
    class GLENGINE_EXPORT FlexView : public View {
    public:
        FlexView(Alignment horizontalAlignment, Alignment verticalAlignment);

        enum class Direction {
            Horizontal,
            Vertical
        };

        void SetDirection(Direction direction) { direction_ = direction; }
        void SetGap(float gap) { gap_ = gap; }

        float4 Arrange(float4 available) override;

    private:
        Alignment horizontalAlignment_;
        Alignment verticalAlignment_;
        Direction direction_ = Direction::Horizontal;
        float gap_ = 0;
    };
}
