#pragma once

#include "3d/Pawn.h"


class PilotableCube : public glengine::world::Pawn {
public:
    PilotableCube();

    void Update(double deltaTime) override {};

    void OnPossess(glengine::input::InputManager* manager) override;

    void OnUnpossess() override;
private:
    std::shared_ptr<PilotableCube> previous, next;
};