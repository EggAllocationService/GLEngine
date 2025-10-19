//
// Created by Kyle Smith on 2025-10-18.
//

#pragma once
#include "3d/ActorSceneComponent.h"

class CubeSceneComponent : public glengine::world::ActorSceneComponent {
public:
    void Render() override;
    void Update(double deltaTime) override {};

    float4 Color = float4(1, 0, 0, 1);
    float Scale = 1.0f;

};
