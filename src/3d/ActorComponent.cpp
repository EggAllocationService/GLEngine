//
// Created by Kyle Smith on 2026-05-28.
//
#include "3d/ActorComponent.h"
#include "3d/Actor.h"

namespace glengine::world {
    Engine* ActorComponent::CURRENT_ENGINE_CONSTRUCTING = nullptr;
    Actor* ActorComponent::CURRENT_ACTOR_CONSTRUCTING = nullptr;

    Engine * ActorComponent::GetEngine() const {
        if (actor_ == nullptr) {
            return CURRENT_ENGINE_CONSTRUCTING;
        }
        return actor_->GetEngine();
    }
}
