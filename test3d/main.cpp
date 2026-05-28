//
// Created by Kyle Smith on 2025-10-18.
//

#include <fstream>

#include "Engine.h"
#include "SpinnyCube.h"
#include "3d/mesh/StaticMesh.h"

#define KEY_ESCAPE 27

using namespace glengine;

float frand(float min, float max) {
    float x = rand() / (float)RAND_MAX;

    return min + (x * (max - min));
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char** argv) {
#ifdef _WIN32
    // Hide the console window on Windows
    FreeConsole();
#endif
    glfwSetErrorCallback(error_callback);
    glfwInit();

    auto engine = new Engine("3D Test", int2(1280, 720));
    engine->SetAllowNonFocusedPawnInput(true);

    auto cube = engine->SpawnActor<SpinnyCube>();

    cube->GetTransform()->SetPosition(float3(0, 0, 4));


    engine->GetInputManager()->AddAction(KEY_ESCAPE, [=]() {
            engine->GetMouseManager()->SetMouseMode(glengine::input::MouseMode::FREE);
        });

    engine->GetInputManager()->AddAction('v', [=]() {
            engine->GetMouseManager()->SetMouseMode(glengine::input::MouseMode::CAPTIVE);
        });
    
    engine->MainLoop();
}