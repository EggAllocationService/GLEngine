#pragma once
#include <vector>
#include "Widget.h"
#include "Colors.h"

namespace glengine::widgets {

	/// Lightweight performance counter
	/// Draws the current framerate to the screen, averaged over a second
	/// Clicking on the displayed value toggles between framerate (FPS) and frametime (milliseconds)
	class PerfCounter : public Widget {
	public:
		PerfCounter();

		void Update(double deltaTime) override;
		void Draw(MatrixStack2D& stack) override;
		void Click(int button, int state, float2 pos) override;

		float4 TextColor = Colors::GREEN;
	private:
		/// <summary>
		/// Stores each deltaTime value, to average frametimes over a second
		/// </summary>
		std::vector<double> counts;

		/// <summary>
		/// time since last average
		/// </summary>
		double timer = 0;

		/// Cached formatted strings so we're not making unnecessary allocations every frame
		std::string lastFrametime = "null ms";
		std::string lastFPS = "null FPS";

		/// <summary>
		/// if true, shows FPS
		/// else, shows frametime in ms
		/// </summary>
		bool showInverse = true;
	};
}