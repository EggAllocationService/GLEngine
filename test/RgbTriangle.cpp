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
	button->SetClickListener([this](int type, int state, float2 pos) {
			if (type == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
				this->clickPos = pos + button->GetEffectiveAbsolutePosition();
				bringToFront();
			}
			if (type == GLUT_LEFT_BUTTON && state == GLUT_DRAG) {
				this->Position += (pos - clickPos);
				this->clickPos = pos;
			}
		});
	button->Position = float2(-20, 0);
	button->Anchor = TOP_MIDDLE;
	button->MinimumWidth = 260;
	button->SetSpacing(5, 3);

	auto closeButton = AddChildWidget<widgets::Button>();
	closeButton->Position = float2(0, 0);
	closeButton->Anchor = TOP_RIGHT;
	closeButton->MinimumWidth = 20;
	closeButton->BackgroundColor = float4(1.0, 0.3, 0.3, 1.0);
	closeButton->SetClickListener([this](int button, int state, float2 pos) {if (state == GLUT_UP) this->Destroy();});
	closeButton->SetText("X");
	closeButton->SetSpacing(5, 3);

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

	if (isTop) {
		glColor4fv(Colors::WHITE);
	} else {
		glColor4fv(float4(1.0, 1.0, 1.0, 0.5));
	}
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
}

void RgbTriangle::Click(int button, int state, float2 pos) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		bringToFront();
	}
}

void RgbTriangle::bringToFront() {
	int maxZ = 0;
	for (auto widget : GetEngine().GetWidgetsOfType<RgbTriangle>()) {
		if (widget->ZIndex > maxZ) {
			if (widget->ZIndex > ZIndex) {
				maxZ = widget->ZIndex--;
			}

			widget->isTop = false;
		}
	}

	isTop = true;

	if (ZIndex >= maxZ) {
		return;
	}

	ZIndex = maxZ;
}
