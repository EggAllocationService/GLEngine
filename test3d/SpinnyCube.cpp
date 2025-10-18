//
// Created by Kyle Smith on 2025-10-18.
//

#include "SpinnyCube.h"

#include "Engine.h"

SpinnyCube::SpinnyCube() {
    CreateComponent<CubeSceneComponent>();
    cube = CreateComponent<CubeSceneComponent>();
    cube->GetTransform()->SetPosition(float3(3, 0, 0));
}

void SpinnyCube::Update(double deltaTime) {
    rotation += (3.14159 / 2.0) * deltaTime;

    GetTransform()->SetRotation(float3(3.14159 / 2.0, rotation, 0));
}
