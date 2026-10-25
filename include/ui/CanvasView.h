//
// Created by Kyle Smith on 2026-10-24.
//
#pragma once

#include "ui/UIElement.h"
#include "glengine_export.h"

namespace glengine::ui {
    /// Positions children using pixel offsets and anchor points relative to the view.
    class GLENGINE_EXPORT CanvasView : public View {
    public:
        CanvasView();

        float4 Arrange(float4 available) override;
    };
}
