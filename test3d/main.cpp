//
// Created by Kyle Smith on 2025-10-18.
//

#include "Engine.h"
#include "SpinnyCube.h"
using namespace glengine;

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    auto engine = new Engine("3D Test", int2(1280, 720));

    auto cube = engine->SpawnActor<SpinnyCube>();
    auto cube2 = engine->SpawnActor<SpinnyCube>();

    cube->GetTransform()->SetPosition(float3(-4, 0, 4));
    cube2->GetTransform()->SetPosition(float3(4, 0, 4));

    glutMainLoop();
}