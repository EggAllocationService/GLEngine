//
// Created by Kyle Smith on 2025-11-19.
//

#include "3d/components/DirectionalLightComponent.h"

#include "3d/Actor.h"
#include "Engine.h"
#include "pipeline/RenderObjects.h"

using namespace glengine::world::components;

void DirectionalLightComponent::Update(double x) {
    auto manager = GetActor()->GetEngine()->GetRenderObjectsManager();

    auto dir = GetForwardVector();

    rendering::LightInfo info = {
        .position = float4(dir, 0.0),
        .diffuse = Diffuse * Intensity,
        .specular = Specular * Intensity,
        .ambient = Ambient * Intensity,
    };

    manager->PushLight(info);
}
