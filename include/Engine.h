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

namespace glengine {
    class Engine {
    public:
        Engine(const std::string &windowName, int2 windowSize);

        ~Engine();

        void Render();

        void Update();

        /// <summary>
        /// Target update interval
        /// </summary>
        int maxFPS = 120;

        template <typename T>
        T *AddOnscreenWidget() {
            T *widget = Widget::New<T>(this);
            widgets.push_back(widget);
            return widget;
        }

        int2 GetWindowSize() const {
            return windowSize;
        }

    private:
        int2 windowSize;
        int windowHandle;

        std::chrono::steady_clock::time_point lastUpdate;

        /// <summary>
        /// Gets the number of seconds since the last Update() call.
        /// This will almost always be a very small number
        /// </summary>
        double calculateDeltaTime();

        void clearBuffers();

        void setLastUpdate();

        void renderWidgets();

        void updateWidgets(double deltaTime);

        std::vector<Widget *> widgets;
    };
} // glengine

#endif //GLENGINE_ENGINE_H
