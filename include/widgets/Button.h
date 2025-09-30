#pragma once

#include "Widget.h"
#include <string>

namespace glengine::widgets {
	class Button : public Widget {
	public:
		Button();

		virtual void Draw(MatrixStack2D& stack) override;

		virtual void Click(int button, int state) override;

		void SetText(std::string newText);

		float4 BackgroundColor = float4(1, 1, 1, 1);

		float4 TextColor = float4(0, 0, 0, 1);

		float4 ShadowColor = float4(0, 0, 0, 0.3);

		/// <summary>
		/// Padding between shadow/border and text
		/// </summary>
		int Padding;

		/// <summary>
		/// Width of border/shadow
		/// </summary>
		int Border;
	protected:
		virtual float2 CalculateSize() const;

		virtual void Update(double DeltaTime) override;

		std::string Text;
		
	};
};