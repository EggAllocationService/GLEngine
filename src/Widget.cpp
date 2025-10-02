//
// Created by Kyle Smith on 2025-09-26.
//

#include "Widget.h"

#include "Engine.h"
#include "GLMath.h"
#include <algorithm>

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

	// remove children marked for destroy
	std::erase_if(children,
		[](std::shared_ptr<Widget> child) {return child->IsDestroyed();});

	// sort children by z-index
	std::sort(children.begin(), children.end(),
		[](std::shared_ptr<Widget>& a, std::shared_ptr<Widget>& b) {
			return a->ZIndex < b->ZIndex;
		});
}

/**
 * Computes an offset for anchoring a child rect within a parent rect
 * This method tries to make the midpoints match: e.g. MIDDLE_LEFT will have the midpoint of both
 * child and parent rect's left edge touching
 *
 * @param anchor where to anchor the child
 * @param bounds size of the parent
 * @param size size of the child
 * @return offset to add to the child's position to achieve the anchoring
 */
float2 calculateAnchorOffset(glengine::Anchoring anchor, float2 bounds, float2 size) {
	int y_alignment = anchor / 3;
	int x_alignment = anchor % 3;
	float2 result;

	if (x_alignment == 0) { // LEFT x alignment
		result.x = 0;
	} else if (x_alignment == 1) { // MIDDLE x alignment
		result.x = (bounds.x / 2) - (size.x / 2);
	} else if (x_alignment == 2) { // RIGHT x alignment
		result.x  = bounds.x - size.x;
	}

	if (y_alignment == 0) { // BOTTOM y alignment
		result.y = 0;
	} else if (y_alignment == 1) { // MIDDLE y alignment
		result.y = (bounds.y / 2) - (size.y / 2);
	} else if (y_alignment == 2) { // TOP y alignment
		result.y  = bounds.y - size.y;
	}

	return result;
}

mat3 glengine::Widget::GetTransformMatrix() const {
	mat3 translate = mat3::identity();
	float2 pos = GetEffectiveRelativePosition();

	// set transform
	translate[2]->set(0, pos.x);
	translate[2]->set(1, pos.y);


	mat3 rotation = math::rotation2D(Rotation);

	return translate * rotation;
}

float2 glengine::Widget::GetEffectiveRelativePosition() const {
	return Position + calculateAnchorOffset(this->Anchor, GetEnclosingBounds(), this->Bounds);
}

float2 glengine::Widget::GetEffectiveAbsolutePosition() const {
	auto base = GetEffectiveRelativePosition();

	// Go up until we reach the window itself, adding relative positions along the way
	Widget *current = parent;
	while (current != nullptr) {
		base += current->GetEffectiveRelativePosition();
		current = current->GetParent();
	}

	return base;
}

float2 glengine::Widget::GetEnclosingBounds() const {
	return (parent == nullptr) ? float2(engine->GetWindowSize()) : parent->Bounds;
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
