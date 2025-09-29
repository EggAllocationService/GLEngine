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
		/// Positions this widget, in pixels relative to parent.
		/// 0,0 is bottom left
		/// </summary>
        int2 Position;

        /// <summary>
		/// Width and height of this widget, in pixels
		/// </summary>
        float2 Scale;

        /// <summary>
		/// Clockwise rotation, in radians.
		/// </summary>
        float Rotation;

        virtual void Update(double DeltaTime) = 0;

        virtual void Draw(MatrixStack2D &stack) = 0;

        mat3 GetTransformMatrix(int2 canvasSize) const;
    };
}

#endif //GLENGINE_WIDGET_H