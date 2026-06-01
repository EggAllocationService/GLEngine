//
// Created by Kyle Smith on 2025-10-18.
//

#include "SpinnyCube.h"

#include <fstream>

#include "Colors.h"
#include "Engine.h"
#include "GLMath.h"
#include "../include/3d/text/SlugTextComponent.h"
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
    model_ = model;
    model_->SetMesh(mesh);
    model_->SetupAttachment(root->GetTransform());

    auto text = CreateComponent<glengine::world::font::SlugTextComponent>();
    text_ = text;
    text_->GetTransform()->SetPosition(float3(0, 0, 2));

    auto font = GetEngine()
        ->GetResourceManager()
        ->GetResource<glengine::world::font::Font>("Builtin_FiraCode");
    text_->SetFont(font);
    text_->SetText("Hello World!");
    model_->GetTransform()->SetPosition(float3(2, 0, 0));

}

void SpinnyCube::Update(double deltaTime) {
    rotation += (PI / 2.0) * deltaTime;

    root->GetTransform()->SetRotation(float3(0, rotation, 0));
    auto a = abs(sin(rotation));
    model_->GetTransform()->SetScale(float3(a, a, a));

}
