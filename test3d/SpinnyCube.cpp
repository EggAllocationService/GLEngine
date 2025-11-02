//
// Created by Kyle Smith on 2025-10-18.
//

#include "SpinnyCube.h"

#include <fstream>

#include "CubeSceneComponent.h"
#include "Colors.h"
#include "Engine.h"
#include "3d/mesh/StaticMeshComponent.h"
#include "3d/components/PointLightComponent.h"
using namespace glengine::world::mesh;
SpinnyCube::SpinnyCube() {
    root = CreateComponent<glengine::world::ActorPrimitiveComponent>();

    auto model = CreateComponent<StaticMeshComponent>();
    auto mesh = GetEngine()
        ->GetResourceManager()
        ->GetResource<StaticMesh>("../test3d/assets/enterprise.obj");
    model->SetMesh(mesh);

    auto light = CreateComponent<glengine::world::components::PointLightComponent>();

    light->SetupAttachment(root->GetTransform());
    light->GetTransform()->SetPosition(float3(0, 0, 5));

    auto cube2 = CreateComponent<CubeSceneComponent>();

    cube2->Scale = 0.3;
    cube2->SetupAttachment(light->GetTransform());
}

void SpinnyCube::Update(double deltaTime) {
    rotation += (3.14159 / 2.0) * deltaTime;

    root->GetTransform()->SetRotation(float3(0, rotation, 0));
}
