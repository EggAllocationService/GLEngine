//
// Created by Kyle Smith on 2025-10-17.
//
#pragma once
#include "ActorComponent.h"
#include "ActorPrimitiveComponent.h"
#include "Transform.h"

namespace glengine::world {
    /// ActorSceneComponents are actor components that have a transform and may be rendered on screen
    class ActorSceneComponent : public ActorPrimitiveComponent {
    public:
        virtual void Render() = 0;
    };
}
