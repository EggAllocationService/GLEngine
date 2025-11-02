#include "3d/components/PointLightComponent.h"

#include "3d/Actor.h"
#include "Engine.h"
#include "pipeline/RenderObjects.h"

using namespace glengine::rendering;
using namespace glengine::world::components;

void PointLightComponent::Update(double) {
	auto manager = GetActor()->GetEngine()->GetRenderObjectsManager();

	auto pos = GetAbsolutePosition();

	LightInfo info = {
		.position = float4(pos, 1.0),
		.diffuse = Diffuse,
		.specular = Specular
	};

	manager->PushLight(info);
}