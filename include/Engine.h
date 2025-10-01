//
// Created by Kyle Smith on 2025-09-26.
//

#ifndef GLENGINE_ENGINE_H
#define GLENGINE_ENGINE_H
#include <string>
#include <vector>
#include <chrono>
#include "Vectors.h"
#include "Widget.h"
#include "MouseManager.h"

namespace glengine {
    class Engine {
    public:
        Engine(const std::string &windowName, int2 windowSize);

        ~Engine();

        void Render();

        void Update();

        void Quit() {
            quitRequested = true;
        }

        /// <summary>
        /// Target update interval
        /// </summary>
        int MaxFPS = 120;

        template <typename T>
        std::shared_ptr<T> AddOnscreenWidget() {
            static_assert(std::is_base_of_v<Widget, T>, "T must be derive from Widget");

            std::shared_ptr<T> widget = Widget::New<T>(this);
            widgets.push_back(widget);
            return widget;
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

        input::MouseManager* GetMouseManager() {
            return mouseManager;
        }

    private:
        int2 windowSize;
        int windowHandle;

        std::chrono::steady_clock::time_point lastUpdate;

        /// When true, causes the engine to destroy the window and exit on the next frame.
        bool quitRequested = false;

        /// <summary>
        /// Gets the number of seconds since the last Update() call.
        /// This will almost always be a very small number
        /// </summary>
        double calculateDeltaTime();

        void clearBuffers();

        void setLastUpdate();

        void renderWidgets();

        void updateWidgets(double deltaTime);

        input::MouseManager* mouseManager;

        std::vector<std::shared_ptr<Widget>> widgets;
    };
} // glengine

#endif //GLENGINE_ENGINE_H
