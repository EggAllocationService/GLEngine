//
// Created by Kyle Smith on 2026-10-24.
//

#include "ui/PerfCounter.h"

#include <cstdio>

#include "Engine.h"
#include "3d/text/Font.h"

namespace glengine::ui {
    PerfCounter::PerfCounter() {
        // note: placement is the parent's job; don't bake in a position offset
        size = float2(260, 32);
        padding = float4(8, 6, 8, 6);
        backgroundColor = float4(0, 0, 0, 0.55f);
        // purely informational; never blocks clicks to what's underneath
        mouseInteraction = MouseInteraction::Ignore;

        label_ = AddElement<TextBlock>();
        label_->SetFontSize(16.0f);
    }

    void PerfCounter::OnAddedToEngine() {
        label_->SetFont(engine_->GetResourceManager()->GetResource<world::font::Font>("/builtin/fonts/quattrocento.ttf"));
    }

    void PerfCounter::Update(double deltaTime) {
        // smooth the frame time so the readout is stable
        const float dt = static_cast<float>(deltaTime);
        smoothedDelta_ = smoothedDelta_ <= 0.0f ? dt : smoothedDelta_ * 0.95f + dt * 0.05f;

        const auto times = engine_->GetLastPerformanceTimes();
        char text[128];
        snprintf(text, sizeof(text), "%.0f fps  (u %.2f ms, r %.2f ms)",
                 1.0f / smoothedDelta_, times.update, times.render);
        label_->SetText(text);

        View::Update(deltaTime);
    }
}
