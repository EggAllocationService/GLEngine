//
// Created by Kyle Smith on 2025-10-01.
//
#pragma once
#include <memory>
#include "Vectors.h"

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
        void Click(int button, int action);

        /// Called every frame to check that
        void Update(double DeltaTime);

        /// Sets the active mouse mode
        void SetMouseMode(MouseMode mode);

        std::shared_ptr<Widget> GetHoveredWidget() {
            return hoveredWidget.lock();
        }

    private:
        Engine* engine;
        std::weak_ptr<Widget> hoveredWidget;

        float2 mousePosition;
        bool clicking = false;

        MouseMode mouseMode = FREE;

        void centerCursor() const;

        std::shared_ptr<Widget> FindHoveredWidget(float2 position) const;
    };
}
