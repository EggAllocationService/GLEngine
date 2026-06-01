//
// Created by Kyle Smith on 2025-10-18.
//
#pragma once

#include "3d/Actor.h"
#include "3d/ActorPrimitiveComponent.h"
#include "3d/mesh/InstancedStaticMeshComponent.h"
#include "3d/mesh/StaticMeshComponent.h"
#include "3d/text/SlugTextComponent.h"


class SpinnyCube : public glengine::world::Actor {
public:
    SpinnyCube();
    void Update(double deltaTime) override;

private:
    float rotation = 0.0f;
    std::shared_ptr<glengine::world::ActorPrimitiveComponent> root;
    std::shared_ptr<glengine::world::font::SlugTextComponent> text_;
    std::shared_ptr<glengine::world::mesh::InstancedStaticMeshComponent> model_;
};
