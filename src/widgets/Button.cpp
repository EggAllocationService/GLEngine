#include "widgets/Button.h"
#include "engine_GLUT.h"
#include "Colors.h"

glengine::widgets::Button::Button()
{
	padding = 3;
	border = 1;
	pressed = false;
}

void glengine::widgets::Button::Update(double DeltaTime)
{

}

void glengine::widgets::Button::Draw(MatrixStack2D& stack)
{
	
	// draw background
	if (pressed) {
		// if the button is pressed, then render everything with the shadow color
		glColor4fv(float4(Colors::blend(BackgroundColor, ShadowColor).rgb, 1.0));
	}
	else {
		glColor4fv(BackgroundColor);
	}
	stack.DrawRect(float2(0, 0), Bounds);

	// draw text
	glColor4fv(TextColor);
	stack.PrintText(float2(border + padding, border + padding), Text.c_str());
	glFlush();

	// setup blending and set shadow color, if not pressed
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4fv(ShadowColor);

	if (!pressed) {
		// draw bottom edge shadow
		stack.DrawRect(float2(border, 0), float2(Bounds.x, border));

		// draw right edge shadow
		stack.DrawRect(float2(Bounds.x - border, 0), Bounds - float2(0, border));
	}
	else {
		// draw top edge shadow
		stack.DrawRect(float2(border, Bounds.y - border), float2(Bounds.x - border, Bounds.y));

		// draw left edge shadow
		stack.DrawRect(float2(0, border), float2(border, Bounds.y));
	}
}

void glengine::widgets::Button::Click(int button, int state, float2 position)
{
	if (Toggle) {
		if (state == GLUT_UP) {
			pressed = !pressed;
			onClick(button, pressed ? GLUT_DOWN : GLUT_UP);
		}
	}
	else {
		pressed = (state == GLUT_DOWN);
		if (onClick != nullptr) {
			onClick(button, state);
		}
	}
}

void glengine::widgets::Button::SetText(std::string newText)
{
	Text = newText;
	Bounds = CalculateSize();
}

void glengine::widgets::Button::SetSpacing(int padding, int border)
{
	this->padding = padding;
	this->border = border;

	Bounds = CalculateSize();
}

float2 glengine::widgets::Button::CalculateSize() const
{
	const int characterWidth = 8; // using 8x13 font
	const int characterHeight = 10;

	return float2(
		// x = (Border + Padding) + (characterWidth * string length) + (Border + Padding)
		(2.0 * (border + padding)) + (characterWidth * Text.length()),

		// y = (Border + Padding) + characterHeight + (Border + Padding)
		(2.0 * (border + padding)) + characterHeight
	);
}

