//
// Created by Kyle Smith on 2025-09-26.
//

#include "Matrix.h"
#include "widgets/Button.h"
#include <iostream>
#include "Engine.h"
#include "engine_GLUT.h"
#include "RgbTriangle.h"

using namespace glengine;

int main(int argc, char **argv) {
    glutInit(&argc, argv);

    auto inst = new Engine("Hello World!", int2(1200, 600));
    inst->maxFPS = 165;

    auto widget = inst->AddOnscreenWidget<RgbTriangle>();
    auto button = inst->AddOnscreenWidget<widgets::Button>();

    button->SetText("Click Me!");
    button->Border = 2;
    button->Padding = 5;

    button->Position = float2(100, 100);

    glutMainLoop();

}
