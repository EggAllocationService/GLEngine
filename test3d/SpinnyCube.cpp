//
// Created by Kyle Smith on 2025-10-18.
//

#include "SpinnyCube.h"

#include <fstream>

#include "CubeSceneComponent.h"
#include "Colors.h"
#include "Engine.h"
#include "3d/components/DirectionalLightComponent.h"
#include "3d/mesh/StaticMeshComponent.h"
#include "3d/components/PointLightComponent.h"
#include "3d/texture/StaticTexture2D.h"
using namespace glengine::world::mesh;
using namespace glengine::world::texture;

SpinnyCube::SpinnyCube() {
    root = CreateComponent<glengine::world::ActorPrimitiveComponent>();

    auto model = CreateComponent<StaticMeshComponent>();
    auto texture = GetEngine()
        ->GetResourceManager()
        ->GetResource<StaticTexture2D>("/Users/kyle/Downloads/assignment_files_updated/sand.jpg");

    auto mesh = GetEngine()
        ->GetResourceManager()
        ->GetResource<StaticMesh>("/Users/kyle/Downloads/assignment_files_updated/submarine - updated.obj");
    model->SetMesh(mesh);
    model->SetTexture(texture);

    auto light = CreateComponent<glengine::world::components::PointLightComponent>();

    light->SetupAttachment(root->GetTransform());
    light->GetTransform()->SetPosition(float3(0, 0, 3));
    light->Intensity = 10;

    this->light = light;

    auto cube2 = CreateComponent<StaticMeshComponent>();
    cube2->SetMesh(mesh);
    cube2->material.Emissive = float4(1, 1, 1 , 1);
    cube2->SetupAttachment(light->GetTransform());
    cube2->GetTransform()->SetScale(float3(0.2, 0.2, 0.2));
}

void SpinnyCube::Update(double deltaTime) {
    rotation += (3.14159 / 2.0) * deltaTime;

    root->GetTransform()->SetRotation(float3(0, rotation, 0));
}
