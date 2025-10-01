#include "RgbTriangle.h"
#include "Colors.h"
#include "Engine.h"
#include "engine_GLUT.h"
#include "GLMath.h"

RgbTriangle::RgbTriangle() {
	hue = 0;
	size = int2(300, 300);
	Bounds = float2(300, 350);
	innerRotation = 0;

	button = AddChildWidget<widgets::Button>();

	button->SetText("Drag Me!");
	button->SetClickListener([this](int button, int state, float2 pos) {
			if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
				this->clickPos = pos;
				pause = true;
			}
			if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
				this->Position += (pos - clickPos);
				pause = false;
			}
		});
	button->Position = float2(0, 0);
	button->Anchor = TOP_MIDDLE;
	button->MinimumWidth = 300;
	button->SetSpacing(5, 3);

	Bounds.y = 300 + button->Bounds.y + 10;
}

void RgbTriangle::Update(double DeltaTime) {
	if (pause) {
		return;
	}

	hue += DeltaTime * 120;
	if (hue > 360) {
		hue = 0;
	}

	innerRotation += rotationScalar * DeltaTime * (3.14159 / 2);
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
