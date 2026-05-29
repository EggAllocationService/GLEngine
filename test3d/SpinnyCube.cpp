//
// Created by Kyle Smith on 2025-10-18.
//

#include "SpinnyCube.h"

#include <fstream>

#include "Colors.h"
#include "Engine.h"
#include "GLMath.h"
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
        ->GetResource<StaticMesh>("/Users/kyle/Downloads/assignment_files_updated/submarine - updated.obj");
    model->SetMesh(mesh);
    model->SetupAttachment(root->GetTransform());
}

void SpinnyCube::Update(double deltaTime) {
    rotation += (PI / 2.0) * deltaTime;

    root->GetTransform()->SetRotation(float3(0, rotation, 0));
}
