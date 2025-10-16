//
// Created by Kyle Smith on 2025-09-26.
//
#pragma once

#include <string>
#include <vector>
#include <ranges>
#include <chrono>
#include "Vectors.h"
#include "Widget.h"
#include "MouseManager.h"

namespace glengine {
    class Engine {
    public:
        Engine(const std::string &windowName, int2 windowSize);

        ~Engine();

        /// Renders all renderable objects to the screen
        void Render();

        /// Simulates the game world
        void Update();

        void Quit() {
            quitRequested = true;
        }

        /// Creates a new widget of type T and adds it to the viewport
        /// Returns a shared reference to the created widget
        template <typename T>
        std::shared_ptr<T> AddOnscreenWidget() {
            static_assert(std::is_base_of_v<Widget, T>, "T must be derive from Widget");

            std::shared_ptr<T> widget = Widget::New<T>(this);
            widgets.push_back(widget);
            return widget;
        }

        /// Helper function to find all widgets of a given type
        template <typename T>
        auto GetWidgetsOfType() {
            static_assert(std::is_base_of_v<Widget, T>, "T must be derive from Widget");

            return std::ranges::views::transform(widgets, [](std::shared_ptr<Widget> widget) {return dynamic_pointer_cast<T>(widget); })
                | std::ranges::views::filter([](std::shared_ptr<T> transformed) {return transformed != nullptr; });
        }

        int2 GetWindowSize() const {
            return windowSize;
        }

        /// Call to update internal states whenever the window size changes
        void SetWindowSize(int2 size);

        /// Get a reference to the list of active widgets
        std::vector<std::shared_ptr<Widget>>& GetWidgets() {
            return widgets;
        }

        input::MouseManager* GetMouseManager() const {
            return mouseManager;
        }

    private:
        int2 windowSize;
        int windowHandle;

        // used for tracking deltaTime
        std::chrono::steady_clock::time_point lastUpdate;

        /// When true, causes the engine to destroy the window and exit on the next frame.
        bool quitRequested = false;

        /// <summary>
        /// Gets the number of seconds since the last Update() call.
        /// This will almost always be a very small number
        /// </summary>
        double calculateDeltaTime() const;

        void clearBuffers();

        void setLastUpdate();

        void renderWidgets();

        void updateWidgets(double deltaTime);

        input::MouseManager* mouseManager;

        std::vector<std::shared_ptr<Widget>> widgets;
    };
} // glengine