#pragma once

#include "Widget.h"
#include <string>
#include <functional>

namespace glengine::widgets {
	class Button : public Widget {
	public:
		Button();

		void Draw(MatrixStack2D& stack) override;

		void Click(int button, int state, float2 pos) override;

		void SetText(std::string newText);
		
		void SetSpacing(int padding, int border);

		void SetClickListener(std::function<void(int button, int state, float2 pos)> listener) {
			onClick = listener;
		}

		bool IsPressed() const {
			return pressed;
		}

		float4 BackgroundColor = float4(1, 1, 1, 1);

		float4 TextColor = float4(0, 0, 0, 1);

		float4 ShadowColor = float4(0, 0, 0, 0.3);

		/// <summary>
		/// Whether this button should act as a toggle button.
		/// 
		/// When true, the click listener `state` parameter will be GLUT_DOWN if the button is toggled on, or GLUT_UP otherwise.
		/// </summary>
		bool Toggle = false; 

	protected:
		virtual float2 CalculateSize() const;

		virtual void Update(double DeltaTime) override;

		std::string Text;
		
		std::function<void(int button, int state, float2 pos)> onClick;

		/// <summary>
		/// Padding between shadow/border and text
		/// </summary>
		int padding;

		/// <summary>
		/// Width of border/shadow
		/// </summary>
		int border;

		bool pressed;
	};
};