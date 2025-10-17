//
// Created by Kyle Smith on 2025-10-17.
//
#pragma once
#include <vector>

#include "ActorComponent.h"
#include "Transform.h"

/// Any object that can be represented in the 3D world.
namespace glengine::world {
    class Actor {
    public:
        virtual ~Actor() = default;

        virtual void Update(double deltaTime) = 0;

        mat4 GetTransformMatrix() const {
            return static_cast<mat4>(transform_);
        }

    private:
        Transform transform_;
        std::vector<std::shared_ptr<ActorComponent>> components_;
    };
}

