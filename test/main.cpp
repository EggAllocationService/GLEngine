//
// Created by Kyle Smith on 2025-09-26.
//

#include "Matrix.h"
#include "widgets/Button.h"
#include <iostream>
#include "Engine.h"
#include "engine_GLUT.h"
#include "RgbTriangle.h"
#include <Colors.h>

using namespace glengine;

int main(int argc, char **argv) {
    glutInit(&argc, argv);

    auto inst = new Engine("Hello World!", int2(1200, 600));
    inst->maxFPS = 60;

    auto widget = inst->AddOnscreenWidget<RgbTriangle>();
    auto button = inst->AddOnscreenWidget<widgets::Button>();

    button->SetText("Click Me!");
    button->SetSpacing(5, 2);
    button->SetClickListener([=](int kind, int state) {
            if (kind == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
                int hue = (rand() / (float)RAND_MAX) * 360;
                button->BackgroundColor = Colors::hsv(hue, 1.0, 1.0);

                button->SetText(std::format("Hue: {}", hue));
            }
        });

    button->Position = float2(100, 100);
    button->Toggle = true;
    button->ZIndex = 100;

    glutMainLoop();

}
