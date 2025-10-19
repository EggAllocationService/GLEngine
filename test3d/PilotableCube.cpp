#include "PilotableCube.h"
#include "CubeSceneComponent.h"
#include "Engine.h";

using namespace glengine;

PilotableCube::PilotableCube()
{
	auto box = CreateComponent<CubeSceneComponent>();
	box->Color = float4(0, 1, 1, 1);
	box->Scale = 0.4;
	defaultCamera->GetTransform()->SetPosition(float3(0, 0.5, -2.5));
	defaultCamera->GetTransform()->SetRotation(float3(3.14159 / 8.0, 0, 0));
}

void PilotableCube::OnPossess(input::InputManager* manager)
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

	manager->AddAction('e', [this]() {
		if (next == nullptr) {
			next = GetEngine()->SpawnActor<PilotableCube>();
			next->GetTransform()->SetPosition(GetTransform()->GetPosition());
			next->previous = dynamic_pointer_cast<PilotableCube>(this->shared_from_this());
		}
		GetEngine()->Possess(next);
		});
	manager->AddAction('q', [this]() {
		if (previous != nullptr) {
			GetEngine()->Possess(previous);
		}
		});
}

void PilotableCube::OnUnpossess()
{
}
