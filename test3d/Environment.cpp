//
// Created by Kyle Smith on 2026-06-03.
//

#include "Environment.h"

#include "Engine.h"
#include "3d/components/DirectionalLightComponent.h"
#include "GLMath.h"
#include "3d/components/AxesComponent.h"
#include "3d/mesh/StaticMeshComponent.h"

Environment::Environment() {
    CreateComponent<glengine::world::components::AxesComponent>();
    auto sun = CreateComponent<glengine::world::components::DirectionalLightComponent>();
    sun->GetTransform()->SetRotation({-PI / 4, PI / 4, 0});
    sun->Ambient = float4(0.1, 0.1, 0.1, 1);
    sun->Diffuse = float4(1, 1, 1, 1);

    auto floor = CreateComponent<glengine::world::mesh::StaticMeshComponent>();
    floor->SetMesh("/builtin/models/plane.obj");
    floor->material->Diffuse = float4(0, 0.3, 0, 1);
    floor->GetTransform()->SetPosition({0, -1, 0});

    auto light = CreateComponent<glengine::world::components::PointLightComponent>();
    light->Diffuse = float4(1, 0, 0, 1);
    light->Ambient = float4(0.1, 0, 0, 1);
    light->Intensity = 5;

    light->GetTransform()->SetPosition({3, 1, 0});

    auto sphere = CreateComponent<glengine::world::mesh::StaticMeshComponent>();
    sphere->SetMesh("/builtin/models/sphere.obj");
    sphere->GetTransform()->SetScale({0.3, 0.3, 0.3});
    sphere->material->Ambient = float4(1, 0, 0, 1);

    sphere->SetupAttachment(light->GetTransform());
}

void Environment::Update(double deltaTime) {
}
