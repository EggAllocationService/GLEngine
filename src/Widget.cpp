//
// Created by Kyle Smith on 2025-09-26.
//

#include "Widget.h"

#include "GLMath.h"

glengine::Widget::Widget() {
	Position = float2(0, 0);
	Rotation = 0;
	Scale = float2(1, 1);
}

void glengine::Widget::UpdateAll(double DeltaTime) {
	Update(DeltaTime);
	for (auto child : children) {
		child->UpdateAll(DeltaTime);
	}
}

mat3 glengine::Widget::GetTransformMatrix() const {
	mat3 scale = mat3::identity();
	mat3 translate = mat3::identity();

	// set transform
	translate[2]->set(0,  Position.x);
	translate[2]->set(1, Position.y);

	// set scales
	scale[0]->set(0, Scale.x );
	scale[1]->set(1, Scale.y );

	mat3 rotation = math::rotation2D(Rotation);

	return translate * scale * rotation;
}

glengine::Engine& glengine::Widget::GetEngine() const {
	return *engine;
}

void glengine::Widget::RenderChildren(MatrixStack2D &stack) const {
	for (auto child : children) {
		stack.Push(child->GetTransformMatrix());
		child->Draw(stack);
		stack.Pop();
	}
}
