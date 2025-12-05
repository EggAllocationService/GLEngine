//
// Created by Kyle Smith on 2025-10-01.
//
#pragma once
#include <memory>
#include "Vectors.h"
#include "glengine_export.h"

// for mouse drag events
#define GLUT_UP 1
#define GLUT_DOWN 2
#define GLUT_DRAG 3
#define GLUT_LEFT_BUTTON 4
#define GLUT_RIGHT_BUTTON 5

namespace glengine {
    class Engine;
    class Widget;
}

namespace glengine::input {
    /// How the engine is handling mouse input
    enum GLENGINE_EXPORT MouseMode {
        /// The user is free to use the mouse. Hover and click events will be passed to widgets
        FREE,

        /// The mouse is captured, and relative input will be passed to the engine
        /// Used for i.e. a first person camera
        CAPTIVE
    };

    /// Handles mouse input from GLUT
    class GLENGINE_EXPORT MouseManager {
    public:
        explicit MouseManager(Engine* engine);

        /// Passive motion callback.
        void HandleMotion(float2 position);

        /// GLUT click callback
        void Click(int button, int action, float2 pos);

        /// Sets the active mouse mode
        void SetMouseMode(MouseMode mode);

        /// Gets the last reported mouse position, in window x/y coordinates
        [[nodiscard]] float2 GetMousePosition() const {
            return mousePosition;
        }

        [[nodiscard]] MouseMode GetMouseMode() const {
            return mouseMode;
        }

    private:
        Engine* engine;

        float2 mousePosition;

        // this essentially freezes the value of `hoveredWidget` while true
        // the idea is that you can mousedown on a widget, move the mouse outside the widget, then mouseup
        // and the widget will still receive the mouseup event since `clicking` is true the whole time.
        bool clicking = false;

        MouseMode mouseMode = FREE;

        // teleport the cursor to the center of the window
        void centerCursor() const;
    };
}
