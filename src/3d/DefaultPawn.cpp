//
// Created by Kyle Smith on 2025-10-18.
//

#include "DefaultPawn.h"

void glengine::world::DefaultPawn::OnPossess(input::InputManager* manager)
{
	manager->AddMouseAxis([this](float2 delta) {
		auto rotation = this->GetTransform()->GetRotation();
		rotation.xy -= float2(delta.yx) * 0.0005;
		this->GetTransform()->SetRotation(rotation);
		});

	manager->AddAxis('w', 's', [this](float amount) {
		auto position = this->GetTransform()->GetPosition();
		auto forward = this->GetTransform()->GetForwardVector();
		position = position + float3(forward.x, 0.0, forward.z).norm() * amount;
		this->GetTransform()->SetPosition(position);

		});

	manager->AddAxis('a', 'd', [this](float amount) {
		auto position = this->GetTransform()->GetPosition();
		position = position + this->GetTransform()->GetLeftVector() * amount;
		this->GetTransform()->SetPosition(position);

		});

	manager->AddAxis(' ', 'c', [this](float amount) {
		auto position = this->GetTransform()->GetPosition();
		position = position + float3(0, 1, 0) * amount;
		this->GetTransform()->SetPosition(position);

		});
}
