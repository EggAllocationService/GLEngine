//
// Created by Kyle Smith on 2026-06-09.
//

#include "3d/actors/DirectionalLightActor.h"

#include "Engine.h"
#include "3d/objects/LightTracker.h"
#include "pipeline/RenderObjects.h"

glengine::world::actors::DirectionalLightActor::DirectionalLightActor() = default;

void glengine::world::actors::DirectionalLightActor::Update(double deltaTime) {
    auto dir = GetTransform()->GetForwardVector();

    rendering::LightInfo info = {
        .position = float4(dir, 0.0),
        .diffuse = Diffuse * Intensity,
        .specular = Specular * Intensity,
        .ambient = Ambient * Intensity,
    };

    GetEngine()->GetRenderObjectsManager()->GetObject<objects::LightTracker>()->AddLight(info);
}
