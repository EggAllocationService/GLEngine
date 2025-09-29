//
// Created by Kyle Smith on 2025-09-26.
//

#include "Widget.h"

glengine::Widget::Widget() {
	Position = float2(0, 0);
	Rotation = 0;
	Scale = float2(1, 1);
}

mat3 glengine::Widget::GetTransformMatrix(int2 canvasSize) const {
	mat3 scale = mat3::identity();
	mat3 translate = mat3::identity();

	// set transform
	translate[2]->set(0,  (2.0f * Position.x) / canvasSize.x);
	translate[2]->set(1, (2.0f * Position.y) / canvasSize.y);

	// set scales
	scale[0]->set(0, 2.0f * Scale.x / canvasSize.x);
	scale[1]->set(1, 2.0f * Scale.y / canvasSize.y);
	scale[2]->set(0, -1.0f);
	scale[2]->set(1, -1.0f);

	float trigValues[9] = {cos(Rotation), sin(Rotation), 0, -sin(Rotation), cos(Rotation), 0, 0, 0, 1};
	mat3 rotationMatrix = mat3(trigValues);

	return translate * scale * rotationMatrix;
}

glengine::Engine& glengine::Widget::GetEngine() const {
	return *engine;
}
