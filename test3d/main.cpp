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

float frand(float min, float max) {
    float x = rand() / (float)RAND_MAX;

    return min + (x * (max - min));
}

int main(int argc, char** argv) {
#ifdef _WIN32
    // Hide the console window on Windows
    FreeConsole();
#endif

    glutInit(&argc, argv);

    auto engine = new Engine("3D Test", int2(1280, 720));

    auto cube = engine->SpawnActor<SpinnyCube>();

    cube->GetTransform()->SetPosition(float3(0, 0, 4));
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

    engine->GetConsole()->AddConsoleCommand("add", [=](std::string_view x) {
        int amount = 100;
        if (x.length() > 0) {
            std::from_chars(x.data(), x.data() + x.length(), amount);
        }

        for (int i = 0; i < amount; i++) {
            auto actor = engine->SpawnActor<SpinnyCube>();
            actor->GetTransform()->SetPosition(float3(
                frand(-50, 50),
                frand(-50, 50),
                frand(-50, 50)
            ));
        }
    });
    
    glutMainLoop();
}