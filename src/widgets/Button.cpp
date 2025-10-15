#include "widgets/Button.h"
#include "engine_GLUT.h"
#include "Colors.h"
#include "MouseManager.h"

glengine::widgets::Button::Button() {
	padding = 3;
	border = 1;
	pressed = false;

	// very annoying... GLUT on Windows doesn't let you use the default "click" cursor.
	// this works correct on macOS, but can't find any constant that looks right for Windows
	Cursor = GLUT_CURSOR_INFO;
}

void glengine::widgets::Button::Update(double DeltaTime) {
	// buttons don't really do anything on their own
}

void glengine::widgets::Button::Draw(MatrixStack2D &stack) {
	// draw background
	if (pressed) {
		// if the button is pressed, then render everything with the shadow color
		glColor4fv(float4(Colors::blend(BackgroundColor, ShadowColor).rgb, 1.0));
	} else {
		glColor4fv(BackgroundColor);
	}
	stack.DrawRect(float2(0, 0), Bounds);

	// calculate the width of just the text, in case we have MinimumWidth set
	// this lets us center the text even when the button is very wide
	float textWidth = 8 * text.length();
	float textPosX = Bounds.x / 2 - textWidth / 2;

	// draw text
	glColor4fv(TextColor);
	stack.PrintText(float2(textPosX, border + padding), text.c_str());

	// setup blending and set shadow color
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4fv(ShadowColor);

	if (pressed) {
		// draw top edge shadow
		stack.DrawRect(float2(border, Bounds.y - border), float2(Bounds.x - border, Bounds.y));

		// draw left edge shadow
		stack.DrawRect(float2(0, border), float2(border, Bounds.y));
	} else {
		// draw bottom edge shadow
		stack.DrawRect(float2(border, 0), float2(Bounds.x, border));

		// draw right edge shadow
		stack.DrawRect(float2(Bounds.x - border, 0), Bounds - float2(0, border));
	}
}

void glengine::widgets::Button::Click(int button, int state, float2 pos) {
	if (Toggle) {
		// if this is a toggle button, then send a click event representing if it's toggled on or off
		// GLUT_UP for off (raised button)
		// GLUT_DOWN for on (depressed button)
		if (state == GLUT_UP && onClick != nullptr) { // change state on mouse up
			pressed = !pressed;
			onClick(button, pressed ? GLUT_DOWN : GLUT_UP, pos);
		}
	} else {
		// since we're not a toggle button, just forward events to the delegate
		// also set `pressed` so that we can render a pressed state
		pressed = (state == GLUT_DOWN || state == GLUT_DRAG);
		if (onClick != nullptr) {
			onClick(button, state, pos);
		}
	}
}

void glengine::widgets::Button::SetText(std::string newText) {
	text = newText;
	Bounds = CalculateSize();
}

void glengine::widgets::Button::SetSpacing(int padding, int border) {
	this->padding = padding;
	this->border = border;

	Bounds = CalculateSize();
}

float2 glengine::widgets::Button::CalculateSize() const {
	const int characterWidth = 8; // using 8x13 font
	const int characterHeight = 10; // slightly underestimate font height for more visually accurate padding

	auto result = float2(
		// x = (Border + Padding) + (characterWidth * string length) + (Border + Padding)
		(2.0 * (border + padding)) + (characterWidth * text.length()),

		// y = (Border + Padding) + characterHeight + (Border + Padding)
		(2.0 * (border + padding)) + characterHeight
	);
	result.x = std::max(result.x, MinimumWidth);

	return result;
}

