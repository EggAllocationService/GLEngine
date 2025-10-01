//
// Created by Kyle Smith on 2025-09-26.
//

#include "Matrix.h"
#include "widgets/Button.h"
#include "widgets/PerfCounter.h"
#include <iostream>
#include "Engine.h"
#include "engine_GLUT.h"
#include "RgbTriangle.h"
#include <Colors.h>

using namespace glengine;

int main(int argc, char **argv) {
    glutInit(&argc, argv);

    auto inst = new Engine("Hello World!", int2(1200, 600));
    inst->MaxFPS = 999;

    auto widget = inst->AddOnscreenWidget<RgbTriangle>();
    auto button = inst->AddOnscreenWidget<widgets::Button>();
    auto counter = inst->AddOnscreenWidget<widgets::PerfCounter>();
    counter->Position = float2(10, 10);

    button->SetText("Click Me!");
    button->SetSpacing(5, 2);
    button->SetClickListener([=](int kind, int state) {
            if (kind == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
                int hue = (rand() / (float)RAND_MAX) * 360;
                button->BackgroundColor = Colors::hsv(hue, 1.0, 1.0);

                button->SetText(std::format("Hue: {}", hue));
            }
        });

    button->Position = float2(0, 100);
    button->Toggle = true;
    button->Anchor = BOTTOM_MIDDLE;
    button->ZIndex = 100;

    glutMainLoop();

}
