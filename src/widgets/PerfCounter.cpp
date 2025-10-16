#include "widgets/PerfCounter.h"
#include "engine_GLUT.h"
#include <format>

glengine::widgets::PerfCounter::PerfCounter() {
	Cursor = GLUT_CURSOR_INFO;

	// 8 wide characters, room for 10
	Bounds = float2(8 * 10, 13);
}

void glengine::widgets::PerfCounter::Update(double deltaTime) {
	// add the deltaTime onto the list and increment the timer
	counts.push_back(deltaTime);
	timer += deltaTime;

	// every second, average the contents of `counts` then reset it
	if (timer > 1.0) {
		// first, reset the timer
		timer = 0.0;

		const size_t len = counts.size();
		double total = 0.0;

		for (const double frameTime: counts) {
			total += frameTime;
		}

		total /= len;

		lastFrametime = std::format("{:.2f}ms", total * 1000);
		lastFPS = std::format("{:.2f} FPS", 1.0 / total);

		counts.clear();
	}
}

void glengine::widgets::PerfCounter::Draw(MatrixStack2D &stack) {
	glColor4fv(TextColor);

	stack.PrintText(float2(0, 0), (showInverse ? lastFrametime : lastFPS).c_str());
}

void glengine::widgets::PerfCounter::Click(int button, int state, float2 pos) {
	if (state == GLUT_UP) {
		showInverse = !showInverse;
	}
}
