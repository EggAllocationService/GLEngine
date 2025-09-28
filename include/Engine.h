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
        /// Gets the number of seconds since the last Update() call.
        /// This will almost always be a very small number
        /// </summary>
        double DeltaTime();

        /// <summary>
        /// Target update interval
        /// </summary>
        int maxFPS = 60;

        void AddOnscreenWidget(Widget* widget);

    private:
        int2 windowSize;
        int windowHandle;
        float hue;

        std::chrono::steady_clock::time_point lastUpdate;

        void clearBuffers();
        void setLastUpdate();

        void renderWidgets();

        std::vector<Widget*> widgets;

    };
} // glengine

#endif //GLENGINE_ENGINE_H