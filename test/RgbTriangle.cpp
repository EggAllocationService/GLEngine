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

	Position.x = sin(Rotation);

}

void RgbTriangle::Draw(MatrixStack2D& stack)
{
	

	std::vector<float3> verticies = {
		float3(-0.866, -0.5, 1.0),
		float3(0.866, -0.5, 1.0),
		float3(0.0, 1.0, 1.0)
	};

	glColor4fv(Colors::hsv(360 - hue, 1.0, 1.0));
	stack.DrawRect(float2(-1.0, -1.0), float2(1.0, 1.0));

	glColor4fv(Colors::hsv(hue, 1.0, 1.0));
	stack.DrawPolygon(verticies);


	glFlush();
}
