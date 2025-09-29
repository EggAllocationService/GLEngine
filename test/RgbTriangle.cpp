#include "RgbTriangle.h"
#include "Colors.h"
#include "Engine.h"
#include "engine_GLUT.h"
#include "GLMath.h"

RgbTriangle::RgbTriangle() {
	hue = 0;
	Position = float2(300, 300);
	size = float2(150, 150);
	velocity = float2(80, 90);
	innerRotation = 0;
}

void RgbTriangle::Update(double DeltaTime) {
	hue += DeltaTime * 120;
	if (hue > 360) {
		hue = 0;
	}

	innerRotation += DeltaTime * (3.14159 / 2);

	Position += velocity * DeltaTime;
	if (Position.x + size.x > GetEngine().GetWindowSize().x || Position.x - size.x < 0) {
		velocity.x = -velocity.x;
	}

	if (Position.y + size.y > GetEngine().GetWindowSize().y || Position.y - size.y < 0) {
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
	stack.DrawRect(-size, size);

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
	stack.Push(math::scale2D(float2(0.5, 0.5)) * math::rotation2D(-2 * innerRotation));

	glBegin(GL_POLYGON);
	i = 0;
	for (float3 &vertex: verticies) {
		glColor4fv(Colors::hsv(((int) hue + (90 * i)) % 360, 1.0, 1.0));
		glVertex3fv(stack * (vertex * float3(150, 150, 1.0)));
		i++;
	}
	glEnd();

	stack.Pop();
	stack.Pop();

	glFlush();
}
