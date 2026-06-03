//
// Created by Kyle Smith on 2026-06-03.
//

#include "Environment.h"

#include "Engine.h"
#include "3d/components/DirectionalLightComponent.h"
#include "GLMath.h"
#include "3d/mesh/StaticMesh.h"
#include "3d/mesh/StaticMeshComponent.h"

Environment::Environment() {
    auto sun = CreateComponent<glengine::world::components::DirectionalLightComponent>();
    sun->GetTransform()->SetRotation({-PI / 4, PI / 4, 0});
    sun->Ambient = float4(0.1, 0.1, 0.1, 1);
    sun->Diffuse = float4(1, 1, 1, 1);

    auto mesh = GetEngine()->GetResourceManager()->GetResource<glengine::world::mesh::StaticMesh>("enterprise.obj");

    auto model = CreateComponent<glengine::world::mesh::StaticMeshComponent>();
    model->SetMesh(mesh);
    model->GetTransform()->SetRotation({PI / 4, PI / 4, 0});
    model->GetTransform()->SetScale({0.5, 0.5, 0.5});

}

void Environment::Update(double deltaTime) {
}
