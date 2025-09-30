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
		/// Width and height of this widget, in pixels.
        /// This is used for hit-testing mouse events
		/// </summary>
        float2 Bounds;

        /// <summary>
		/// Clockwise rotation, in radians.
		/// </summary>
        float Rotation;

        /// <summary>
        /// Z-index for rendering/hit-testing order. Higher value means more to the front.
        /// 
        /// Items on the same Z-index are rendered in arbitrary order.
        /// </summary>
        int ZIndex;

        /**
         * Updates this widget and all children
         * @param DeltaTime Number of seconds since last update. Usually very small.
         */
        virtual void UpdateAll(double DeltaTime);

        /**
         * Render this widget to the screen
         * @param stack a matrix stack to help position vertices in an appropriate location
         */
        virtual void Draw(MatrixStack2D &stack) = 0;

        /**
         * Handle a button click within the bounds of the widget
         * @see Widget::Bounds
         */
        virtual void Click(int button, int state);

        mat3 GetTransformMatrix() const;

        /**
         * Create a new widget instance for T, owned by `engine`
         */
    	template<typename T>
    	static T *New(Engine *engine) {
    		static_assert(std::is_base_of_v<Widget, T>, "T must derive from Widget");
    		T *result = new T();
    		result->engine = engine;

            for (auto child : result->GetChildren()) {
                // make sure to setup engine reference correctly for child widgets added in constructor
                child->engine = engine;
            }
    		return result;
    	}

        /**
         * Add a child widget to this.
         */
    	template<typename T>
	    std::shared_ptr<T> AddChildWidget() {
    		std::shared_ptr<T> widget = std::shared_ptr<T>(New<T>(engine));
    		widget->parent = std::shared_ptr<Widget>(this);
    		children.push_back(widget);
    		return widget;
    	}

    	const std::vector<std::shared_ptr<Widget>>& GetChildren() const {
    		return children;
    	}

    protected:
    	/// <summary>
    	/// Get the Engine that owns this Widget
    	/// </summary>
    	Engine& GetEngine() const;

        /**
         * Get the parent widget
         * @return The parent widget, or null if none exists
         */
        std::shared_ptr<Widget> GetParent() const {
    		return parent;
    	}

        /**
         * Renders all child widgets using the given matrix stack
         */
        void RenderChildren(MatrixStack2D &stack) const;

        /**
         * Update this widget, but not any child widgets
         * @param DeltaTime number of seconds that have passed since the last call. Usually very small.
         */
        virtual void Update(double DeltaTime) = 0;

    private:
    	Engine *engine;
    	std::vector<std::shared_ptr<Widget>> children;
    	std::shared_ptr<Widget> parent = nullptr;
    };
}

#endif //GLENGINE_WIDGET_H