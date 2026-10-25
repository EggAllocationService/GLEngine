//
// Created by Kyle Smith on 2026-10-24.
//
#pragma once

#include "ui/UIElement.h"
#include "ui/TextBlock.h"
#include "glengine_export.h"

namespace glengine::ui {
    /// A reusable widget that displays update/render times and FPS.
    /// Add to any view in one line: engine->AddUIElement<ui::PerfCounter>();
    class GLENGINE_EXPORT PerfCounter : public View {
    public:
        PerfCounter();

        void OnAddedToEngine() override;
        void Update(double deltaTime) override;

    private:
        std::shared_ptr<TextBlock> label_;
        float smoothedDelta_ = 0.0f;
    };
}
