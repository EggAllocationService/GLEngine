//
// Created by Kyle Smith on 2025-10-18.
//

#include "Engine.h"
#include "SpinnyCube.h"
#include "widgets/PerfCounter.h"
#include "widgets/Button.h"

using namespace glengine;

int main(int argc, char** argv) {
#ifdef _WIN32
    // Hide the console window on Windows
    FreeConsole();
#endif
    
    glutInit(&argc, argv);

    auto engine = new Engine("3D Test", int2(1280, 720));

    auto cube = engine->SpawnActor<SpinnyCube>();

    cube->GetTransform()->SetPosition(float3(0, 0, 4));

    auto cube2 = engine->SpawnActor<SpinnyCube>();
    cube2->GetTransform()->SetPosition(float3(1, 1, 4));
    cube2->GetTransform()->SetScale(float3(0.5, 0.5, 0.5));

    auto counter = engine->AddOnscreenWidget<widgets::PerfCounter>();
    counter->Anchor = TOP_LEFT;

    engine->GetInputManager()->AddAction(27, [=]() {
            engine->GetMouseManager()->SetMouseMode(glengine::input::MouseMode::FREE);
        });

    engine->GetInputManager()->AddAction('v', [=]() {
            engine->GetMouseManager()->SetMouseMode(glengine::input::MouseMode::CAPTIVE);
        });
    
    glutMainLoop();
}