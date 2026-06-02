//
// Created by Kyle Smith on 2025-10-18.
//

#include "SpinnyCube.h"

#include <fstream>
#include <unistd.h>

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

    auto mesh = GetEngine()
        ->GetResourceManager()
        ->GetResource<StaticMesh>("cube-tex.obj");
    model_ = model;
    model_->SetMesh(mesh);
    model_->SetupAttachment(root->GetTransform());

    auto text = CreateComponent<glengine::world::font::SlugTextComponent>();
    text_ = text;
    text_->GetTransform()->SetPosition(float3(0, 0, -1.5));

    auto font = GetEngine()
        ->GetResourceManager()
        ->GetResource<glengine::world::font::Font>("/builtin/QuattrocentoSans.ttf");
    text_->SetFont(font);
    text_->SetText("Amazingly few discothèques provide jukeboxes.");

}

void SpinnyCube::Update(double deltaTime) {
    rotation += (PI / 2.0) * deltaTime * 0.1;
    root->GetTransform()->SetRotation(float3(0, rotation, 0));
}
