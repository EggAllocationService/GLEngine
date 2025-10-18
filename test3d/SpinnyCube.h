//
// Created by Kyle Smith on 2025-10-18.
//
#pragma once
#include "CubeSceneComponent.h"
#include "3d/Actor.h"


class SpinnyCube : public glengine::world::Actor {
public:
    SpinnyCube();
    void Update(double deltaTime) override;

private:
    float rotation = 0.0f;
    std::shared_ptr<CubeSceneComponent> cube;
};
