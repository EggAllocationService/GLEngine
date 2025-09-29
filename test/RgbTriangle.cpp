#include "RgbTriangle.h"
#include "Colors.h"
#include "Engine.h"
#include "engine_GLUT.h"
#include "GLMath.h"

RgbTriangle::RgbTriangle() {
	hue = 0;
	Position = float2(300, 300);
	size = int2(150, 150);
	velocity = float2(80, 90);
	innerRotation = 0;
}

void RgbTriangle::Update(double DeltaTime) {
	int2 bounds;
	if (GetParent() != nullptr) {
		auto parentTri = dynamic_cast<RgbTriangle*>(GetParent().get());
		if (parentTri != nullptr) {
			bounds = parentTri->size;
		} else {
			bounds = GetEngine().GetWindowSize();
		}
	} else {
		bounds = GetEngine().GetWindowSize();
	}

	hue += DeltaTime * 120;
	if (hue > 360) {
		hue = 0;
	}

	innerRotation += DeltaTime * (3.14159 / 2);

	Position += velocity * DeltaTime;
	if (Position.x + size.x > bounds.x || Position.x - size.x < 0) {
		velocity.x = -velocity.x;
	}

	if (Position.y + size.y > bounds.y || Position.y - size.y < 0) {
		velocity.y = -velocity.y;
	}
}

void RgbTriangle::Draw(MatrixStack2D &stack) {
	std::vector verticies = {
		float3(-1.0, 0, 1.0),
		float3(0.0, -1.0, 1.0),
		float3(1.0, 0, 1.0),
		float3(0.0, 1.0, 1.0)
	};

	glColor4fv(Colors::WHITE);
	stack.DrawRect(float2(-size), float2(size));

	stack.Push(math::rotation2D(innerRotation)); // push into inner rotation space

	glBegin(GL_POLYGON);
	int i = 0;
	for (float3 &vertex: verticies) {
		glColor4fv(Colors::hsv(((int) hue + (90 * i)) % 360, 1.0, 1.0));
		glVertex3fv(stack * (vertex * float3(150, 150, 1.0)));
		i++;
	}
	glEnd();

	// make another inner rotating square
	stack.Push( math::scale2D(float2(0.5, 0.5)) * math::rotation2D(-2 * innerRotation));

	RenderChildren(stack);

	stack.Pop();
	stack.Pop();

	glFlush();
}
