//
// Created by Kyle Smith on 2026-06-03.
//

#include "Environment.h"

#include "Engine.h"
#include "3d/components/DirectionalLightComponent.h"
#include "GLMath.h"
#include "MeshDemoComponent.h"
#include "3d/components/AxesComponent.h"
#include "3d/mesh/OptimizedMeshComponent.h"
#include "3d/mesh/StaticMeshComponent.h"

Environment::Environment() {
    CreateComponent<glengine::world::components::AxesComponent>();
    auto sun = CreateComponent<glengine::world::components::DirectionalLightComponent>();
    sun->GetTransform()->SetRotation({-PI / 4, PI / 4, 0});
    sun->Ambient = float4(0.1, 0.1, 0.1, 1);
    sun->Diffuse = float4(1, 1, 1, 1);

    for (int x = -5; x < 5; x++) {
        for (int z = -5; z < 5; z++) {
            auto floor = CreateComponent<glengine::world::mesh::OptimizedMeshComponent>("/assets/statue.mesh");
            floor->GetTransform()->SetPosition({(float)x, -1, (float)z});
            floor->GetTransform()->SetRotation({-PI/2, 0, 0});
        }
    }

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
