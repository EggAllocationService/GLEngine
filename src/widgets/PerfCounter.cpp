#include "widgets/PerfCounter.h"
#include "engine_GLUT.h"
#include <format>
#include "Engine.h"

glengine::widgets::PerfCounter::PerfCounter() {
	Cursor = GLUT_CURSOR_INFO;

	// 8 wide characters, room for 20 chars
	// 3 lines of text @ 13px/line
	Bounds = float2(8 * 20, 13 * 3); 
	lastFPS = (char*)calloc(20, sizeof(char));
	lastFrametime = (char*)calloc(20, sizeof(char));
	lastTimes = (char*)calloc(100, sizeof(char));
}

glengine::widgets::PerfCounter::~PerfCounter() {
	free(lastFPS);
	free(lastFrametime);
}

void glengine::widgets::PerfCounter::Update(double deltaTime) {
	// add the deltaTime onto the list and increment the timer
	counts.push_back(deltaTime);

	auto prev = GetEngine()->GetLastPerformanceTimes();
	times.emplace_back(prev.render, prev.update);

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

		snprintf(lastFrametime, 20, "%.2f ms", total * 1000.0);
		snprintf(lastFPS, 20, "%.2f FPS", 1.0 / total);

		counts.clear();

		// average times

		double totalRender = 0;
		double totalUpdate = 0;

		for (const auto& pair : times) {
			totalRender += pair.first;
			totalUpdate += pair.second;
		}

		totalRender /= len;
		totalUpdate /= len;

		snprintf(lastTimes, 100, "Update: %.2f ms\nRender: %.2f ms", totalUpdate, totalRender);
		times.clear();

	}
}

void glengine::widgets::PerfCounter::Draw(MatrixStack2D &stack) {
	glColor4f(0, 0, 0, 0.5);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	stack.DrawRect(float2(0, 0), Bounds);
	glDisable(GL_BLEND);

	glColor4fv(TextColor);

	stack.PrintText(float2(0, 13 * 2), (showInverse ? lastFPS : lastFrametime));
	stack.PrintText(float2(0, 13), lastTimes);
}

void glengine::widgets::PerfCounter::Click(int button, int state, float2 pos) {
	if (state == GLUT_UP) {
		showInverse = !showInverse;
	}
}
