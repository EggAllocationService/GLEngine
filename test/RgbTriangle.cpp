#include "RgbTriangle.h"
#include "Colors.h"
#include "Engine.h"
#include "engine_GLUT.h"
#include "GLMath.h"

RgbTriangle::RgbTriangle() {
	hue = 0;
	size = int2(300, 300);
	Bounds = float2(300, 350);
	velocity = float2(50, 65);
	innerRotation = 0;

	button = AddChildWidget<widgets::Button>();

	button->SetText("Reverse Direction");
	button->SetClickListener([this](int button, int state) {
			if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
				this->rotationScalar *= -1.0;
			}
		});
	button->Position = float2(0, 0);
	button->Anchor = TOP_MIDDLE;
	button->SetSpacing(5, 3);
}

void RgbTriangle::Update(double DeltaTime) {
	int2 bounds = GetEngine().GetWindowSize();

	hue += DeltaTime * 120;
	if (hue > 360) {
		hue = 0;
	}

	innerRotation += rotationScalar * DeltaTime * (3.14159 / 2);

	Position += velocity * DeltaTime;
	if (Position.x + Bounds.x > bounds.x || Position.x < 0) {
		velocity.x = -velocity.x;
	}

	if (Position.y + Bounds.y > bounds.y || Position.y < 0) {
		velocity.y = -velocity.y;
	}
}

void RgbTriangle::Draw(MatrixStack2D &stack) {
	std::vector verticies = {
		float3(0.5, 1.0, 1.0),
		float3(1.0, 0.5, 1.0),
		float3(0.5, 0, 1.0),
		float3(0.0, 0.5, 1.0)
	};

	glColor4fv(Colors::WHITE);
	stack.DrawRect(float2(0, 0), float2(size));


	stack.Push(math::translate2D(float2(150, 150)) * math::rotation2D(innerRotation)); // push into inner rotation space
	stack.Push(math::translate2D(float2(-150, -150)) * math::scale2D(float2(size))); // square offset & scaling
	glBegin(GL_POLYGON);
	int i = 0;
	for (float3 &vertex: verticies) {
		glColor4fv(Colors::hsv((int(hue) + (90 * i)) % 360, 1.0, 1.0));
		glVertex3fv(stack * vertex);
		i++;
	}
	glEnd();
	stack.Pop();

	glColor4fv(Colors::BLACK);
	stack.PrintText(float2(0, 0), "Hello World!");
	stack.Pop();

	RenderChildren(stack);

	glFlush();
}
