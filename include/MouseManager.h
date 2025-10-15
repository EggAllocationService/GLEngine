//
// Created by Kyle Smith on 2025-10-01.
//
#pragma once
#include <memory>
#include "Vectors.h"

// for mouse drag events
// probably fine ish since
#define GLUT_DRAG 2

namespace glengine {
    class Engine;
    class Widget;
}

namespace glengine::input {
    /// How the engine is handling mouse input
    enum MouseMode {
        /// The user is free to use the mouse. Hover and click events will be passed to widgets
        FREE,

        /// The mouse is captured, and relative input will be passed to the engine
        /// Used for i.e. a first person camera
        CAPTIVE
    };

    /// Handles mouse input from GLUT
    class MouseManager {
    public:
        explicit MouseManager(Engine* engine);

        /// Passive motion callback.
        void HandleMotion(float2 position);

        /// GLUT click callback
        void Click(int button, int action, float2 pos);

        /// Called every frame to send hover and drag events
        void Update();

        /// Sets the active mouse mode
        void SetMouseMode(MouseMode mode);

        /// Gets the currently hovered widget, which may be null
        [[nodiscard]] std::shared_ptr<Widget> GetHoveredWidget() const {
            return hoveredWidget.lock();
        }

        /// Gets the last reported mouse position, in window x/y coordinates
        [[nodiscard]] float2 GetMousePosition() const {
            return mousePosition;
        }

    private:
        Engine* engine;
        std::weak_ptr<Widget> hoveredWidget;

        float2 mousePosition;

        // this essentially freezes the value of `hoveredWidget` while true
        // the idea is that you can mousedown on a widget, move the mouse outside the widget, then mouseup
        // and the widget will still receive the mouseup event since `clicking` is true the whole time.
        bool clicking = false;

        MouseMode mouseMode = FREE;

        // teleport the cursor to the center of the window
        void centerCursor() const;

        [[nodiscard]] std::shared_ptr<Widget> hitTestWidgets(float2 position) const;
    };
}
