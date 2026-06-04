//
// Created by Kyle Smith on 2025-10-17.
//
#pragma once
#include "glengine_export.h"

namespace glengine {
    class Engine;
}

namespace glengine::world {
    class Actor;


    /// Base class for any component that can be placed onto an actor
    /// Basic components do not have a scene transform and are not rendered onscreen
    class GLENGINE_EXPORT ActorComponent {
    public:
        static Engine* CURRENT_ENGINE_CONSTRUCTING;
        static Actor* CURRENT_ACTOR_CONSTRUCTING;
        ActorComponent();
        virtual ~ActorComponent() = default;

        virtual void Update(double deltaTime) = 0;

        [[nodiscard]] Actor* GetActor() const {
            return actor_;
        }

        [[nodiscard]] Engine* GetEngine() const;

    private:
        /// Reference to the owning actor for this component
        /// Will always be valid, as an actor will always outlive its components
        Actor* actor_ = nullptr;
    };
}
