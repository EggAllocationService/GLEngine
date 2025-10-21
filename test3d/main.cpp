//
// Created by Kyle Smith on 2025-10-18.
//

#include <fstream>

#include "Engine.h"
#include "SpinnyCube.h"
#include "widgets/PerfCounter.h"
#include "PilotableCube.h"
#include "3d/mesh/StaticMesh.h"
#include "widgets/Button.h"

#define KEY_ESCAPE 27

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
    cube2->GetTransform()->SetRotation(float3(0, 0, -3.15159 / 8));

    auto counter = engine->AddOnscreenWidget<widgets::PerfCounter>();
    counter->Anchor = TOP_LEFT;

    engine->GetInputManager()->AddAction(KEY_ESCAPE, [=]() {
            engine->GetMouseManager()->SetMouseMode(glengine::input::MouseMode::FREE);
        });

    engine->GetInputManager()->AddAction('v', [=]() {
            engine->GetMouseManager()->SetMouseMode(glengine::input::MouseMode::CAPTIVE);
        });

    auto plane = engine->SpawnActor<PilotableCube>();
    engine->Possess(plane);
    
    glutMainLoop();
}