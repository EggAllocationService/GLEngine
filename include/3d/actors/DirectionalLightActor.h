//
// Created by Kyle Smith on 2026-06-09.
//
#pragma once
#include "3d/Actor.h"
#include "glengine_export.h"

namespace glengine::world::actors {
    class DirectionalLightActor : public Actor {
    public:
        GLENGINE_EXPORT DirectionalLightActor();

        void Update(double deltaTime) override;

        float4 Diffuse = float4(0.3, 0.3, 0.3, 1);
        float4 Specular = float4(1, 1, 1, 1);
        float4 Ambient = float4(0, 0, 0, 1);
        float Intensity = 1.0;
    };
}