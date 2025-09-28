//
// Created by Kyle Smith on 2025-09-26.
//

#ifndef GLENGINE_WIDGET_H
#define GLENGINE_WIDGET_H
#include "MatrixStack.h"

namespace glengine {
	/// <summary>
	/// A 2D widget drawn on top of the viewport after 3D rendering.
	/// 
	/// 
	/// </summary>
	class Widget {
	public:
		Widget();

		/// <summary>
		/// Positions this widget. 0,0 is the center  
		/// </summary>
		float2 Position;

		/// <summary>
		/// Scale multiplier for x and y axis
		/// </summary>
		float2 Scale;
		
		/// <summary>
		/// Clockwise rotation, in radians.
		/// </summary>
		float Rotation;

		virtual void Update(double DeltaTime) = 0;
		virtual void Draw(MatrixStack2D& stack) = 0;

		mat3 GetTransformMatrix();
	};
}

#endif //GLENGINE_WIDGET_H