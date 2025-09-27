//
// Created by Kyle Smith on 2025-09-26.
//

#include "Widget.h"


mat3 glengine::Widget::GetTransformMatrix()
{
	mat3 base = mat3::identity();

	// set transform
	base[2]->set(0, Position.x);
	base[2]->set(1, Position.y);
	
	// set scale
	base[0]->set(0, Scale.x);
	base[1]->set(1, Scale.y);

	float trigValues[9] = { cos(Rotation), sin(Rotation), 0, -sin(Rotation), cos(Rotation), 0, 0, 0, 1 };
	mat3 rotationMatrix = mat3(trigValues);

	return base * rotationMatrix;
}