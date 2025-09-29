//
// Created by Kyle Smith on 2025-09-26.
//

#ifndef GLENGINE_WIDGET_H
#define GLENGINE_WIDGET_H
#include "MatrixStack.h"

namespace glengine {
	class Engine;
}

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
        float2 Position;

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

    	template<typename T>
    	static T *New(Engine *engine) {
    		static_assert(std::is_base_of_v<Widget, T>, "T must derive from Widget");
    		T *result = new T();
    		result->engine = engine;
    		return result;
    	}
    protected:
    	/// <summary>
    	/// Get the Engine that owns this Widget
    	/// </summary>
    	Engine& GetEngine() const;
    private:
    	Engine *engine;
    };
}

#endif //GLENGINE_WIDGET_H