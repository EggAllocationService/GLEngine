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

	Rotation += (3.14159 / 2.0) * DeltaTime;

	Scale.xy = abs(sin(Rotation)) + 0.1;

}

void RgbTriangle::Draw(MatrixStack2D& stack)
{
	glColor4fv(Colors::hsv(hue, 1.0, 1.0));

	float3 verticies[3] = {
		float3(-0.866, -0.5, 1.0),
		float3(0.866, -0.5, 1.0),
		float3(0.0, 1.0, 1.0)
	};

	glBegin(GL_POLYGON);
	for (float3& vertex : verticies) {
		glVertex2fv(stack * vertex);
	}
	glEnd();

	glFlush();
}
