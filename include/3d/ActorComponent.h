//
// Created by Kyle Smith on 2025-10-17.
//
#pragma once
#include <memory>

#include "Actor.h"

namespace glengine::world {

    /// Base class for any component that can be placed onto an actor
    /// Basic components do not have a scene transform and are not rendered onscreen
    class ActorComponent {
    public:
        ActorComponent();
        virtual ~ActorComponent() = default;

        virtual void Update(double deltaTime) = 0;
    protected:
        Actor *GetActor() const {
            return actor_;
        }

    private:
        /// Reference to the owning actor for this component
        /// Will always be valid, as an actor will always outlive its components
        Actor* actor_;
    };
}
