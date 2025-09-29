#include "RgbTriangle.h"
#include "Colors.h"
#include "engine_GLUT.h"

RgbTriangle::RgbTriangle() {
	hue = 0;
	Position = int2(300, 300);
}

void RgbTriangle::Update(double DeltaTime) {
	hue += DeltaTime * 120;
	if (hue > 360) {
		hue = 0;
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
	stack.DrawRect(float2(-150, -150), float2(150, 150));

	glBegin(GL_POLYGON);
	int i = 0;
	for (float3 &vertex: verticies) {
		glColor4fv(Colors::hsv(((int) hue + (90 * i)) % 360, 1.0, 1.0));
		glVertex3fv(stack * (vertex * float3(150, 150, 1.0)));

		i++;
	}
	glEnd();

	glFlush();
}
