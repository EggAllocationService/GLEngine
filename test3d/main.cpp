//
// Created by Kyle Smith on 2025-10-18.
//

#include <fstream>

#include "GLMath.h"
#include "Engine.h"
#include "Enterprise.h"
#include "SpinnyCube.h"
#include "../include/3d/text/Font.h"
#include "3d/mesh/StaticMesh.h"

#ifdef _WIN32
#include "windows.h"
#endif

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
    glfwSetErrorCallback(error_callback);
    int result = glfwInit();

#ifdef _WIN32
    // Hide the console window on Windows
    FreeConsole();
#endif

    auto engine = new Engine("3D Test", int2(1280, 720));
    engine->SetAllowNonFocusedPawnInput(true);

    auto cube = engine->SpawnActor<SpinnyCube>();
    cube->GetTransform()->SetPosition(float3(0, 0, 5));

    auto ship = engine->SpawnActor<Enterprise>();


    engine->GetInputManager()->AddAction('x', [=]() {
        auto cube = engine->SpawnActor<SpinnyCube>();
        cube->GetTransform()->SetPosition(float3(frand(-10, 10), 0, frand(-4, 4)));
    });

    engine->GetInputManager()->AddAction(KEY_ESCAPE, [=]() {
            engine->GetMouseManager()->SetMouseMode(glengine::input::MouseMode::FREE);
        });

    engine->GetInputManager()->AddAction('v', [=]() {
            engine->GetMouseManager()->SetMouseMode(glengine::input::MouseMode::CAPTIVE);
        });
    
    engine->MainLoop();
}