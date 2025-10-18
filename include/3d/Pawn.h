#pragma once

#include "components/CameraComponent.h"
#include "Actor.h"

/// Pawns are actors that can be possessed and controlled by the player
namespace glengine::world {
    class Pawn : public Actor {
    public:
        Pawn();

        virtual components::CameraComponent *GetActiveCamera() {
            return defaultCamera.lock().get();
        }
    protected:
        std::weak_ptr<components::CameraComponent> defaultCamera;
    };

}