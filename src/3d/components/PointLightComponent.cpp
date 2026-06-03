#include "3d/components/PointLightComponent.h"

#include "3d/Actor.h"
#include "Engine.h"
#include "3d/objects/LightTracker.h"
#include "pipeline/RenderObjects.h"

using namespace glengine::rendering;
using namespace glengine::world::components;

void PointLightComponent::Update(double) {

	auto pos = GetAbsolutePosition();
	auto manager = GetEngine()->GetRenderObjectsManager()->GetObject<objects::LightTracker>();
	LightInfo info = {
		.position = float4(pos, 1.0),
		.diffuse = Diffuse * Intensity,
		.specular = Specular * Intensity,
		.ambient = Ambient * Intensity,
	};
	manager->AddLight(info);
}