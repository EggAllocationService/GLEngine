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

static int highZ = 10;

using namespace glengine;

int main(int argc, char **argv) {
    glutInit(&argc, argv);

    auto inst = new Engine("Hello World!", int2(1200, 600));

    auto widget = inst->AddOnscreenWidget<RgbTriangle>();
    widget->Anchor = MIDDLE_MIDDLE;

    auto button = inst->AddOnscreenWidget<widgets::Button>();
    auto counter = inst->AddOnscreenWidget<widgets::PerfCounter>();
    counter->Position = float2(10, -10);
    counter->Anchor = TOP_LEFT;

    button->SetText("Add New!");
    button->SetSpacing(5, 2);
    button->SetClickListener([=](int kind, int state, float2 pos) {
            if (kind == GLUT_LEFT_BUTTON && state == GLUT_UP) {
                auto newWidget = inst->AddOnscreenWidget<RgbTriangle>();
                newWidget->Anchor = MIDDLE_MIDDLE;
                newWidget->ZIndex = highZ++;
            }
        });

    button->Position = float2(0, 100);
    button->Anchor = BOTTOM_MIDDLE;
    button->ZIndex = 100;

    auto closeButton = inst->AddOnscreenWidget<widgets::Button>();
    closeButton->Anchor = TOP_RIGHT;
    closeButton->ZIndex = 1000;
    closeButton->BackgroundColor = Colors::RED;
    closeButton->Position = float2(-10, -10);
    closeButton->SetText("Close All");
    closeButton->SetClickListener([=](int button, int state, float2 pos) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
            for (auto widget : inst->GetWidgetsOfType<RgbTriangle>()) {
                widget->Destroy();
            }
        }
    });

    inst->GetInputManager()->AddAction('`', [=] {
       inst->ShowConsole();
    });

    inst->GetConsole()->AddConsoleCommand("aprint", [](const std::string_view x) {
        std::cout << "(print)" << x << std::endl;
    });

    inst->GetConsole()->AddConsoleCommand("clear", [=](const std::string_view x) {
        for (auto widget : inst->GetWidgetsOfType<RgbTriangle>()) {
            widget->Destroy();
        }
    });
    inst->GetConsole()->AddConsoleCommand("add", [=](const std::string_view x) {
        int amount = 1;
        if (x.length() > 0) {
            std::from_chars(x.data(), x.data() + x.length(), amount);
        }

        for (int i = 0; i < amount; i++) {
            auto widget = inst->AddOnscreenWidget<RgbTriangle>();
            widget->Anchor = MIDDLE_MIDDLE;
            widget->ZIndex = highZ++;
            widget->Position = float2(rand() % 30, rand() % 30);
        }
    });

    glutMainLoop();
    delete inst;
}
