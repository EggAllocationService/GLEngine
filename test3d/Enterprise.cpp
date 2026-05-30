//
// Created by Kyle Smith on 2026-05-30.
//

#include "Enterprise.h"

#include "3d/components/CameraComponent.h"
#include "Engine.h"
#include "ResourceManager.h"
#include "GLMath.h"
#include "3d/mesh/InstancedStaticMeshComponent.h"


using namespace glengine::world;
using namespace glengine::world::mesh;

Enterprise::Enterprise()
{
	// put the camera slightly above and behind the middle
	auto rearCamera = CreateComponent<components::CameraComponent>();
	rearCamera->GetTransform()->SetPosition(float3(0, 0.5, -1.5));
	rearCamera->FoV = 80;
	SetActiveCamera(rearCamera);

	// create a root for the mesh, as we need to rotate it 180 degrees but
	// also want to tilt it with movement.
	// this will simplify the tilt code
	auto root = CreateComponent<ActorPrimitiveComponent>();
	root->GetTransform()->SetRotation(float3(0, PI, 0));

	// load mesh
	auto model = GetEngine()
		->GetResourceManager()
		->GetResource<StaticMesh>("enterprise.obj");

	mesh = CreateComponent<InstancedStaticMeshComponent>();
	mesh->SetupAttachment(root->GetTransform());
	mesh->SetMesh(model);
}

inline float clampf(float a, float m, float M) {
	if (a < m) {
		return m;
	}
	if (a > M) {
		return M;
	}
	return a;
}

inline float sign(float a) {
	if (a == 0) {
		return 0;
	}
	return a < 0 ? -1 : 1;
}
inline float2 sign(float2 a) {
	return float2(sign(a.x), sign(a.y));
}

void Enterprise::Update(double deltaTime)
{
	// compute rotation for this frame from tilt
	constexpr float tiltAmount = PI / 16.0; // 22 degrees max
	mesh->GetTransform()->SetRotation((float3(tilt.yx, 0.5 * tilt.x)) * tiltAmount);

	// if either tilt axis is very close to zero, set it to zero to avoid weird oscillations
	if (abs(tilt.x) < deltaTime * 4) {
		tilt.x = 0;
	}
	if (abs(tilt.y) < deltaTime * 4) {
		tilt.y = 0;
	}

	// decay tilt a bit so when all keys are released we gradually go back to straight
	tilt = tilt - (sign(tilt) * deltaTime * (TiltMultiplier / 2.0));
}

void Enterprise::OnPossess(glengine::input::InputManager* input)
{
	input->AddAxis('w', 's', [this](float amount) {
			auto position = this->GetTransform()->GetPosition();
			auto forward = this->GetTransform()->GetForwardVector();
			this->GetTransform()->SetPosition(position + forward * amount);
		});

	// right arrow to go right, left arrow to go left (+X axis = right)
	input->AddAxis('d', 'a', [this](float amount) {
			auto position = this->GetTransform()->GetPosition();
			auto right = this->GetTransform()->GetRightVector();
			this->GetTransform()->SetPosition(position + right * amount);

			// add some tilt input and clamp
			tilt.x += amount * TiltMultiplier;
			tilt.x = clampf(tilt.x, -1, 1);
		});

	// up arrow to go up, down arrow to go down (+Y axis = up)
	input->AddAxis('r', 'f', [this](float amount) {
			auto position = this->GetTransform()->GetPosition();
			this->GetTransform()->SetPosition(position + float3(0, amount, 0));

			// add some tilt input and clamp
			tilt.y += amount * TiltMultiplier;
			tilt.y = clampf(tilt.y, -1, 1);
		});

	// look left/right with keyboard
	input->AddAxis(KEY_ARROW_RIGHT, KEY_ARROW_LEFT, [this](float amount) {
		auto rotation = this->GetTransform()->GetRotation();
		rotation.xy += float2(0, amount);
		this->GetTransform()->SetRotation(rotation);
	});
}