//
// Created by Kyle Smith on 2025-09-26.
//
#pragma once

#include "engine_GLUT.h"
#include "MatrixStack.h"
#include <memory>
#include "glengine_export.h"

namespace glengine {
	class Engine;
}

namespace glengine {

	/// Constants for the various types of anchoring supported
	/// Represents a point on the given edge, i.e. MIDDLE_RIGHT is the midpoint of the right edge
	/// MIDDLE_MIDDLE is the centerpoint of the widget
	enum GLENGINE_EXPORT Anchoring {
		BOTTOM_LEFT,
		BOTTOM_MIDDLE,
		BOTTOM_RIGHT,
		MIDDLE_LEFT,
		MIDDLE_MIDDLE,
		MIDDLE_RIGHT,
		TOP_LEFT,
		TOP_MIDDLE,
		TOP_RIGHT
	};

	enum GLENGINE_EXPORT MouseInteraction {
		/// The widget will block mouse input from widgets below it
		BLOCK,
		/// The widget will be ignored by mouse hit testing
		IGNORED
	};

    /// <summary>
	/// A 2D widget drawn on top of the viewport after 3D rendering.
	/// </summary>
    class GLENGINE_EXPORT Widget {
    public:
        Widget();
    	virtual ~Widget() = default;

        /// <summary>
		/// Positions this widget, in pixels relative to parent.
		/// 0,0 is bottom left
		/// </summary>
        float2 Position;

        /// <summary>
		/// Width and height of this widget, in pixels.
        /// This is used for hit-testing and layout anchoring
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
        /// 
        /// For child widgets, the parent is considered to be -infinity. That is, child widgets will always
        /// be hit-tested before the parent, and rendered on top of the parent
        /// </summary>
        int ZIndex = 0;

    	/// <summary>
    	///	Where the widget's Position is anchored to in the parent.
    	///	The layout algorithm will make the corresponding points line up, so for example
    	///	TOP_MIDDLE will make the midpoint of the top edge of this widget coincident with the parent's top edge midpoint
    	///
    	///	Defaults to BOTTOM_LEFT
    	/// </summary>
    	Anchoring Anchor = BOTTOM_LEFT;

    	/// <summary>
    	///	How this widget should interact with mouse input
    	///
    	///	Defaults to BLOCK
    	/// </summary>
    	MouseInteraction MouseMode = BLOCK;

    	/**
		 * Determines what cursor should be used when hovering this widget
		 *
		 * Must be a GLUT cursor constant. Defaults to `GLUT_CURSOR_INHERIT`
		 */
    	int Cursor = GLUT_CURSOR_INHERIT;

        /**
         * Updates this widget and all children
         * @param deltaTime Number of seconds since last update. Usually very small.
         */
        virtual void UpdateAll(double deltaTime);

        /**
         * Render this widget to the screen
         * @param stack a matrix stack to help position vertices in an appropriate location
         */
        virtual void Draw(MatrixStack2D &stack) = 0;

        /**
         * Handle a button click/drag within the bounds of the widget
         * Note that for drag events, `button` will always be `GLUT_LEFT_BUTTON`.
         * The position parameter might be outside the bounds of the widget when `state` is not `GLUT_DOWN`
         * 
         * @param button the GLUT button constant for the click (e.g. GLUT_LEFT_BUTTON)
         * @param state one of GLUT_DOWN, GLUT_UP, or GLUT_DRAG
         * @param pos where the click happened, relative to the widget's origin
         */
        virtual void Click(int button, int state, float2 pos) {}

        /**
         * Called by the engine when the user's mouse passes enters or leaves the
         * widget's bounds
         *
         * @param hovering whether this widget is being hovered or not
         */
        virtual void HoverStateChanged(bool hovering) {}

        /**
         * Called when keyboard input focus state changes, and this widget was either
         * the previous focus or the new focus
         * @param focused if this widget is focused or not
         */
        virtual void FocusStateChanged(bool focused) {}

        /**
         * Called when a key has been pressed while this widget was focused
         * @param keyCode key that was pressed
         */
        virtual void KeyPressed(int keyCode) {}

        /**
         * Computes a translation and rotation matrix for this widget's position and rotation
         *
         * @return the transform matrix for this widget's position and rotation
         */
        [[nodiscard]] mat3 GetTransformMatrix() const;

        /**
         * Computes the effective relative position of this widget, applying anchoring
         */
        [[nodiscard]] float2 GetEffectiveRelativePosition() const;

        /**
         * Computes the effective absolute window-space position of this widget, applying anchoring
         */
        [[nodiscard]] float2 GetEffectiveAbsolutePosition() const;

        /**
         * Gets the enclosing bounds of this widget
         * @return The parent's `Bounds` field, or the window size if no parent is present.
         */
        float2 GetEnclosingBounds() const;

        /**
         * Marks this widget for deletion
         */
        void Destroy() {
    		pendingKill = true;
    	}

        /**
         * @return If this widget has been marked for deletion
         */
        bool IsDestroyed() const {
    		return pendingKill;
    	}

        /**
         * Create a new widget instance for T, owned by `engine`
         */
    	template<typename T>
    	static std::shared_ptr<T> New(Engine *engine) {
    		static_assert(std::is_base_of_v<Widget, T>, "T must derive from Widget");
    		std::shared_ptr<T> result = std::make_shared<T>();
    		result->engine = engine;

            for (auto child : result->GetChildren()) {
                // make sure to set up engine reference correctly for child widgets added in constructor
                child->engine = engine;
            }
    		return result;
    	}

        /**
         * Creates a new widget and adds it as a child to the current widget
         * @tparam T type of widget to create
         * @return a shared pointer to the newly created widget
         */
        template<typename T>
	    std::shared_ptr<T> AddChildWidget() {
            std::shared_ptr<T> widget = New<T>(engine);
            widget->parent = this;
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
    	Engine* GetEngine() const {
    		return engine;
    	};

        /**
         * Get the parent widget
         * @return The parent widget, or null if none exists
         */
        Widget *GetParent() const {
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
    	Widget *parent = nullptr;

    	bool pendingKill = false;
    };
}
