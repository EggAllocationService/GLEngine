//
// Created by Kyle Smith on 2025-10-27.
//

#include "3d/Actor.h"

namespace glengine::world {
    Engine* CURRENT_ENGINE_CONSTRUCTING = nullptr;

    mat4 Actor::GetTransformMatrix() const {
        if (parent.expired()) {
            return transform_.GetMatrix();
        }
        else {
            auto realParent = parent.lock();
            auto parentActor = realParent->GetActor()->GetTransformMatrix();
            auto parentComponent = realParent->GetTransformMatrix();

            return (parentActor * parentComponent) * transform_.GetMatrix();
        }
    }
}