//
// Created by Kyle Smith on 2025-10-18.
//

#include <fstream>

#include "GLMath.h"
#include "Engine.h"
#include "Enterprise.h"
#include "Environment.h"
#include "MeshDemoComponent.h"
#include "SpinnyCube.h"
#include "../include/3d/text/Font.h"
#include "3d/mesh/StaticMesh.h"

#ifdef _WIN32
#include "windows.h"
#endif

#include "DemoShaders.h"
#include "ui/UIManager.h"
#include "ui/FlexView.h"
#include "ui/Button.h"
#include "ui/PerfCounter.h"

using namespace glengine;

float frand(float min, float max) {
    float x = rand() / (float)RAND_MAX;

    return min + (x * (max - min));
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static bool inverted = false;

int main(int argc, char** argv) {
    glfwSetErrorCallback(error_callback);
    int result = glfwInit();

#ifdef _WIN32
    // Hide the console window on Windows
    FreeConsole();
#endif

    auto engine = new Engine("3D Test", int2(1280, 720));
    engine->GetResourceManager()->MountPak("/assets", "assets.pak");
    engine->SetAllowNonFocusedPawnInput(true);

    MeshDemoComponent::RegisterPipeline(engine);

    auto invert = engine->GetRenderer()->CompilePostEffect(embed_invert_wgsl, 0);
    auto invertIndex = engine->PushPostEffect(invert);
    engine->SetPostEffectEnabled(invertIndex, inverted);

    auto ship = engine->SpawnActor<Enterprise>();
    engine->SetFakePawn(ship);
    engine->Possess(ship);

    engine->SpawnActor<Environment>();

    auto cube = engine->SpawnActor<SpinnyCube>();
    cube->GetTransform()->SetPosition(float3(0, 0, 5));

    engine->GetInputManager()->AddAction('x', [=]() {
        auto possessed = engine->GetPossessedPawn();
        if (possessed == ship.get()) {
            engine->Possess(nullptr);
        } else {
            engine->Possess(ship);
        }
    });

    engine->GetInputManager()->AddAction(KEY_ESCAPE, [=]() {
        engine->GetMouseManager()->SetMouseMode(glengine::input::MouseMode::FREE);
    });

    engine->GetInputManager()->AddAction('v', [=]() {
        engine->GetMouseManager()->SetMouseMode(glengine::input::MouseMode::CAPTIVE);
    });

    engine->GetInputManager()->AddAction('i', [=]() {
        engine->SetPostEffectEnabled(invertIndex, inverted = !inverted);
    });


    auto uiRoot = engine->GetUIManager()->GetRootView();
    auto flex = uiRoot->AddElement<ui::FlexView>(ui::Alignment::Start, ui::Alignment::Start);
    flex->SetDirection(ui::FlexView::Direction::Vertical);
    flex->padding = float4(10, 10, 10, 10);
    flex->AddElement<ui::PerfCounter>();

#ifdef GLENGINE_TEXT_RENDERING
    auto uiFont = engine->GetResourceManager()->GetResource<world::font::Font>("/builtin/fonts/trim.ttf");
#endif

    auto button = flex->AddElement<ui::Button>("Toggle Invert");
    button->size = float2(160, 40);
#ifdef GLENGINE_TEXT_RENDERING
    button->SetFont(uiFont);
#endif
    button->AddClickHandler([engine, invertIndex]() {
        engine->SetPostEffectEnabled(invertIndex, inverted = !inverted);
    });


    engine->MainLoop();
}