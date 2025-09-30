#pragma once
#include <vector>
#include "Widget.h"
#include "Colors.h"

namespace glengine::widgets {
	class PerfCounter : public Widget {
	public:
		PerfCounter();
		virtual void Update(double DeltaTime) override;
		virtual void Draw(MatrixStack2D& stack) override;
		virtual void Click(int button, int state, float2 position) override;

		float4 TextColor = Colors::GREEN;
	private:
		/// <summary>
		/// Stores each DeltaTime value, to average frametimes over a second
		/// </summary>
		std::vector<double> counts;

		/// <summary>
		/// time since last average
		/// </summary>
		double timer = 0;

		/// <summary>
		/// result of last average, in frames / second
		/// </summary>
		double lastFrametime = 0;

		/// <summary>
		/// if true, shows FPS
		/// else, shows frametime in ms
		/// </summary>
		bool showInverse = true;
	};
}