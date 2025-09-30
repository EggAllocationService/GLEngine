#include "widgets/Button.h"
#include "engine_GLUT.h"
#include "Colors.h"

glengine::widgets::Button::Button()
{
	Padding = 3;
	Border = 1;
}

void glengine::widgets::Button::Update(double DeltaTime)
{

}

void glengine::widgets::Button::Draw(MatrixStack2D& stack)
{
	
	// draw background
	glColor4fv(BackgroundColor);
	stack.DrawRect(float2(0, 0), Bounds);

	// draw text
	glColor4fv(TextColor);
	stack.PrintText(float2(Border + Padding, Border + Padding), Text.c_str());
	glFlush();

	// setup blending and set shadow color
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4fv(ShadowColor);

	// draw bottom edge shadow
	stack.DrawRect(float2(1, 1), float2(Bounds.x, Border));

	// draw right edge shadow
	stack.DrawRect(float2(Bounds.x - Border, 0), Bounds);
}

void glengine::widgets::Button::Click(int button, int state)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		BackgroundColor = Colors::hsv((rand() / (float) RAND_MAX) * 360, 1.0, 1.0);
	}
}

void glengine::widgets::Button::SetText(std::string newText)
{
	Text = newText;
	Bounds = CalculateSize();
}

float2 glengine::widgets::Button::CalculateSize() const
{
	const int characterWidth = 8; // using 8x13 font
	const int characterHeight = 13;

	return float2(
		// x = (Border + Padding) + (characterWidth * string length) + (Border + Padding)
		(2.0 * (Border + Padding)) + (characterWidth * Text.length()),

		// y = (Border + Padding) + characterHeight + (Border + Padding)
		(2.0 * (Border + Padding)) + characterHeight
	);
}

