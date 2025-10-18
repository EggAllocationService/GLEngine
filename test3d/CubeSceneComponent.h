//
// Created by Kyle Smith on 2025-10-18.
//

#pragma once
#include "3d/ActorSceneComponent.h"

class CubeSceneComponent : public glengine::world::ActorSceneComponent {
public:
    void Render() override;
    void Update(double deltaTime) override {};
};
