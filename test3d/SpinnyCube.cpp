//
// Created by Kyle Smith on 2025-10-18.
//

#include "SpinnyCube.h"

#include <fstream>

#include "Colors.h"
#include "Engine.h"
#include "GLMath.h"
#include "../src/3d/text/SlugTextComponent.h"
#include "3d/mesh/StaticMeshComponent.h"
#include "3d/components/PointLightComponent.h"
#include "3d/mesh/InstancedStaticMeshComponent.h"
#include "3d/texture/StaticTexture2D.h"
using namespace glengine::world::mesh;
using namespace glengine::world::texture;

SpinnyCube::SpinnyCube() {
    root = CreateComponent<glengine::world::ActorPrimitiveComponent>();

    auto model = CreateComponent<InstancedStaticMeshComponent>();

    const char *meshName = nullptr;
    if (rand() % 2 == 0) {
        meshName = "enterprise.obj";
    } else {
        meshName = "cube-tex.obj";
    }

    auto mesh = GetEngine()
        ->GetResourceManager()
        ->GetResource<StaticMesh>(meshName);
    model->SetMesh(mesh);
    model->SetupAttachment(root->GetTransform());

    auto text = CreateComponent<glengine::world::font::SlugTextComponent>();
    text->GetTransform()->SetPosition(float3(0, 0, 2));
}

void SpinnyCube::Update(double deltaTime) {
    rotation += (PI / 2.0) * deltaTime;

    root->GetTransform()->SetRotation(float3(0, rotation, 0));
}
