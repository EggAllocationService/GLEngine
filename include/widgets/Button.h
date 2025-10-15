#pragma once

#include "Widget.h"
#include <string>
#include <functional>

namespace glengine::widgets {

	/// A simple button widget
	/// Can operate either in standard or toggle mode
	/// In normal mode, click events are passed to the click handler given by SetClickListener
	/// In toggle mode, the button acts like a checkbox and SetClickListener is only called on changes
	class Button : public Widget {
	public:
		Button();

		~Button() override = default;

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
		/// The minimum width of the button.
		/// If the button's width is less than this, padding will be added to make up
		/// </summary>
		float MinimumWidth = 0.0;

		/// <summary>
		/// Whether this button should act as a toggle button.
		/// 
		/// When true, the click listener `state` parameter will be GLUT_DOWN if the button is toggled on, or GLUT_UP otherwise.
		/// </summary>
		bool Toggle = false; 

	protected:
		/// <summary>
		/// Calculates the needed size of the button, given current padding, border, text, and MinimumWidth values
		/// </summary>
		virtual float2 CalculateSize() const;

		void Update(double deltaTime) override;

		std::string text;
		
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