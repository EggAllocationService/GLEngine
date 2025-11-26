//
// Created by Kyle Smith on 2025-10-17.
//
#pragma once
#include "ActorPrimitiveComponent.h"
#include "MatrixStack.h"
#include "pipeline/Renderer.h"

namespace glengine::world {
    /// ActorSceneComponents are actor components that have a transform and may be rendered on screen
    class ActorSceneComponent : public ActorPrimitiveComponent {
    public:
        virtual void Render(Renderer *renderer, MatrixStack& stack) = 0;
    };
}
