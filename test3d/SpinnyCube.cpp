//
// Created by Kyle Smith on 2025-10-18.
//

#include "SpinnyCube.h"

#include "Engine.h"

SpinnyCube::SpinnyCube() {
    cube = CreateComponent<CubeSceneComponent>();
}

void SpinnyCube::Update(double deltaTime) {
    rotation += (3.14159 / 2.0) * deltaTime;

    GetTransform()->SetRotation(float3(0, rotation, 0));

    GetEngine()->GetPossessedPawn()->GetTransform()->SetPosition(float3(0, sin(rotation), 0));
}
