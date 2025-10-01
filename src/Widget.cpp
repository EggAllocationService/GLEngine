//
// Created by Kyle Smith on 2025-09-26.
//

#include "Widget.h"

#include "GLMath.h"

glengine::Widget::Widget() {
	Position = float2(0, 0);
	Bounds = float2(1, 1);
	Rotation = 0;
}

void glengine::Widget::UpdateAll(double DeltaTime) {
	Update(DeltaTime);
	for (auto child : children) {
		child->UpdateAll(DeltaTime);
	}

	// sort children by z-index
	std::sort(children.begin(), children.end(),
		[](std::shared_ptr<Widget>& a, std::shared_ptr<Widget>& b) {
			return a->ZIndex < b->ZIndex;
		});
}

void glengine::Widget::Click(int button, int state, float2 position)
{
	// by default, see if there's any child elements we can pass the event to

	for (auto child : children) {
		if (child->Position < position && position < (child->Position + child->Bounds)) {
			child->Click(button, state, position - child->Position);
		}
	}
}

mat3 glengine::Widget::GetTransformMatrix() const {
	mat3 translate = mat3::identity();

	// set transform
	translate[2]->set(0,  Position.x);
	translate[2]->set(1, Position.y);


	mat3 rotation = math::rotation2D(Rotation);

	return translate * rotation;
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
