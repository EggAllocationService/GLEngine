#include "RgbTriangle.h"
#include "Colors.h"
#include "engine_GLUT.h"

RgbTriangle::RgbTriangle()
{
	hue = 0;
}

void RgbTriangle::Update(double DeltaTime)
{
	hue += DeltaTime * 360;
	if (hue > 360) {
		hue = 0;
	}
}

void RgbTriangle::Draw(MatrixStack2D& stack)
{
	glColor4fv(Colors::hsv(hue, 1.0, 1.0));

	float2 verticies[3] = {
		float2(0.0, 1.0),
		float2(1.0, 0.0),
		float2(-1.0, 0.0)
	};

	glBegin(GL_POLYGON);
	for (float2& vertex : verticies) {
		glVertex2fv(stack * vertex);
	}
	glEnd();

	glFlush();
}
