#pragma once

#include "components/CameraComponent.h"
#include "InputManager.h"
#include "Actor.h"

/// Pawns are actors that can be possessed and controlled by the player
namespace glengine::world {
    class Pawn : public Actor {
    public:
        Pawn();

        /// <summary>
        /// Called when this pawn is possessed - the user's input and view is now directed to this pawn
        /// </summary>
        /// <param name="inputManager">The active input manager, so the pawn can listen to mouse and keyboard input</param>
        virtual void OnPossess(input::InputManager *inputManager) {};

        /// <summary>
        /// Called when the pawn is unpossessed
        /// </summary>
        virtual void OnUnpossess() {};

        virtual components::CameraComponent *GetActiveCamera() {
            return defaultCamera.lock().get();
        }
    protected:
        std::weak_ptr<components::CameraComponent> defaultCamera;
    };

}