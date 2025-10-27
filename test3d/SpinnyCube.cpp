//
// Created by Kyle Smith on 2025-10-18.
//

#include "SpinnyCube.h"

#include <fstream>

#include "CubeSceneComponent.h"
#include "Colors.h"
#include "Engine.h"
#include "3d/mesh/StaticMeshComponent.h"
using namespace glengine::world::mesh;
SpinnyCube::SpinnyCube() {
    root = CreateComponent<glengine::world::ActorPrimitiveComponent>();
    CreateComponent<CubeSceneComponent>();

    mesh = CreateComponent<StaticMeshComponent>();
    auto model = GetEngine()
        ->GetResourceManager()
        ->GetResource<StaticMesh>("../test3d/assets/enterprise.obj");
    mesh->SetMesh(model);

    auto smallerCube = CreateComponent<CubeSceneComponent>();
    smallerCube->SetupAttachment(root->GetTransform());
    smallerCube->Color = Colors::GREEN;

    smallerCube->GetTransform()->SetPosition(float3(0, 0, 3));
    smallerCube->GetTransform()->SetScale(float3(0.4, 0.4, 0.4));

    root2 = CreateComponent<glengine::world::ActorPrimitiveComponent>();
    root2->SetupAttachment(smallerCube->GetTransform());
    root2->GetTransform()->SetRotation(float3(3.141 / 2.0, 0, 0));

    auto evenSmallerCube = CreateComponent<CubeSceneComponent>();
    evenSmallerCube->SetupAttachment(root2->GetTransform());
    evenSmallerCube->Color = float4(1, 1, 0, 1);

    evenSmallerCube->GetTransform()->SetPosition(float3(1.5, 0, 0));
    evenSmallerCube->GetTransform()->SetScale(float3(0.4, 0.4, 0.4));
}

void SpinnyCube::Update(double deltaTime) {
    rotation += (3.14159 / 2.0) * deltaTime;

    mesh->GetTransform()->SetRotation(float3(0, rotation, 0));
    root->GetTransform()->SetRotation(float3(0, rotation, 0));
    root2->GetTransform()->SetRotation(float3(0, rotation, 0));
}
