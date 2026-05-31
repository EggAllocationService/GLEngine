#pragma once
#include "3d/Pawn.h"
#include "3d/mesh/StaticMeshComponent.h"
#include <memory>

#include "3d/mesh/InstancedStaticMeshComponent.h"

class Enterprise : public glengine::world::Pawn {
public:
    Enterprise();

    void Update(double) override;

    void OnPossess(glengine::input::InputManager* inputManager) override;

    /// <summary>
    /// Magic number to multiply tilt amounts by
    /// Higher = faster animation
    ///
    /// Probably equates to a half-tilt taking 1/TiltMultiplier seconds?
    /// </summary>
    float TiltMultiplier = 10.0;

private:
    std::shared_ptr<glengine::world::mesh::StaticMeshComponent> mesh;

    // tilt.x = +right -left
    // tilt.y = +up -down
    float2 tilt;
};
