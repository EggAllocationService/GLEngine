#include "RgbTriangle.h"
#include "Colors.h"
#include "engine_GLUT.h"

RgbTriangle::RgbTriangle() {
	hue = 0;
}

void RgbTriangle::Update(double DeltaTime) {
	hue += DeltaTime * 120;
	if (hue > 360) {
		hue = 0;
	}

	Rotation += (3.14159 / 2.0) * (DeltaTime / 5);

	Scale.xy = abs(sin(Rotation)) + 0.1;

	Position.x = sin(Rotation);
}

void RgbTriangle::Draw(MatrixStack2D &stack) {
	std::vector verticies = {
		float3(-0.866, -0.5, 1.0),
		float3(0.866, -0.5, 1.0),
		float3(0.0, 1.0, 1.0)
	};

	glColor4fv(Colors::WHITE);
	stack.DrawRect(float2(-1.0, -1.0), float2(1.0, 1.0));

	glBegin(GL_POLYGON);
	int i = 0;
	for (float3 &vertex: verticies) {
		glColor4fv(Colors::hsv(((int) hue + (120 * i)) % 360, 1.0, 1.0));
		glVertex3fv(stack * vertex);

		i++;
	}
	glEnd();

	glFlush();
}
