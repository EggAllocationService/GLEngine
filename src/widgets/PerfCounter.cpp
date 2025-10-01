#include "widgets/PerfCounter.h"
#include "engine_GLUT.h"
#include <format>

glengine::widgets::PerfCounter::PerfCounter()
{
	Cursor = GLUT_CURSOR_INFO;

	// 8 wide characters, room for 10
	Bounds = float2(8 * 10, 13);
}

void glengine::widgets::PerfCounter::Update(double DeltaTime)
{
	counts.push_back(DeltaTime);
	timer += DeltaTime;
	
	if (timer > 1.0) {
		timer = 0.0;

		size_t len = counts.size();
		double total = 0.0;

		for (double frameTime : counts) {
			total += frameTime;
		}
		
		total /= len;

		lastFrametime = total;

		counts.clear();
	}
}

void glengine::widgets::PerfCounter::Draw(MatrixStack2D& stack)
{
	glColor4fv(TextColor);

	std::string text;
	if (showInverse) {
		text = std::format("{:.2f} FPS", 1.0 / lastFrametime);
	} 
	else {
		text = std::format("{:.2f}ms", lastFrametime * 1000);
	}

	stack.PrintText(float2(0, 0), text.c_str());
}

void glengine::widgets::PerfCounter::Click(int button, int state) {
	if (state == GLUT_UP) {
		showInverse = !showInverse;
	}
}