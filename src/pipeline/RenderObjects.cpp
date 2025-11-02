#include "pipeline/RenderObjects.h"

#include "engine_GLUT.h"
#include <algorithm>

using namespace glengine::rendering;

RenderObjects::RenderObjects() {
	lights_.reserve(GL_MAX_LIGHTS);
}

void RenderObjects::PushLight(LightInfo l)
{
	// only add if the driver supports that many lights
	if (lights_.size() < GL_MAX_LIGHTS) {
		lights_.push_back(l);
	}
}


void RenderObjects::InitLights() const {
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	for (int i = 0; i < lights_.size(); i++) {
		glEnable(GL_LIGHT0 + i);

		// evil reinterpret_cast which is probably fine since float4 should be four packed floats
		// close your eyes when reading the next few lines
		glLightfv(GL_LIGHT0 + i, GL_POSITION, reinterpret_cast<const float*>(&lights_[i].position));
		glLightfv(GL_LIGHT0 + i, GL_SPECULAR, reinterpret_cast<const float*>(&lights_[i].specular));
		glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, reinterpret_cast<const float*>(&lights_[i].diffuse));
		glLightfv(GL_LIGHT0 + i, GL_AMBIENT, reinterpret_cast<const float*>(&lights_[i].ambient));

		// if its a point light, setup attenuation
		glLightf(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, 0.5);
		glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, 1);
	}

}

void RenderObjects::DeInit() const {
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
	for (int i = 0; i < lights_.size(); i++) {
		glDisable(GL_LIGHT0 + i);
	}
}

void RenderObjects::Reset() {
	lights_.clear();
}
