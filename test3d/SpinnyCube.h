//
// Created by Kyle Smith on 2025-10-18.
//
#pragma once

#include "3d/Actor.h"
#include "3d/ActorPrimitiveComponent.h"
#include "3d/mesh/StaticMeshComponent.h"


class SpinnyCube : public glengine::world::Actor {
public:
    SpinnyCube();
    void Update(double deltaTime) override;

private:
    float rotation = 0.0f;
    std::shared_ptr<glengine::world::ActorPrimitiveComponent> root;
    std::shared_ptr<glengine::world::ActorPrimitiveComponent> root2;
    std::shared_ptr<glengine::world::mesh::StaticMeshComponent> mesh;
};
