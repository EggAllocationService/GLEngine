//
// Created by Kyle Smith on 2025-10-18.
//

#include "DefaultPawn.h"

void glengine::world::DefaultPawn::OnPossess(input::InputManager* manager)
{
	manager->AddMouseAxis([this](float2 delta) {
		auto rotation = this->GetTransform()->GetRotation();
		rotation.xy += float2(-delta.y, delta.x) * 0.0005;
		this->GetTransform()->SetRotation(rotation);
		});

	// w to go forward, s to go back
	manager->AddAxis('w', 's', [this](float amount) {
		auto position = this->GetTransform()->GetPosition();
		auto forward = this->GetTransform()->GetForwardVector();
		position = position + float3(forward.x, 0.0, forward.z).norm() * amount;
		this->GetTransform()->SetPosition(position);

		});

	// d to go right, a to go left
	manager->AddAxis('d', 'a', [this](float amount) {
		auto position = this->GetTransform()->GetPosition();
		position = position + this->GetTransform()->GetRightVector() * amount;
		this->GetTransform()->SetPosition(position);

		});

	// space to go up, c to go down
	manager->AddAxis(' ', 'c', [this](float amount) {
		auto position = this->GetTransform()->GetPosition();
		position = position + float3(0, 1, 0) * amount;
		this->GetTransform()->SetPosition(position);

		});
}

void glengine::world::DefaultPawn::OnUnpossess()
{
	// Don't keep these things around, we found a better pawn to possess
	Destroy();
}
