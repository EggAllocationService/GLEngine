//
// Created by Kyle Smith on 2025-09-26.
//

#ifndef GLENGINE_ENGINE_H
#define GLENGINE_ENGINE_H
#include <string>
#include <vector>
#include "Vectors.h"
#include "Widget.h"

namespace glengine {
    class Engine {
    public:
        Engine(const std::string &windowName, int2 windowSize);
        ~Engine();

        void Render();

        [[nodiscard]] int2 getWindowSize() const {
            return windowSize;
        }

    private:
        int2 windowSize;
        int windowHandle;

        void clearBuffers();

        std::vector<Widget> widgets;

    };
} // glengine

#endif //GLENGINE_ENGINE_H