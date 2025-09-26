//
// Created by Kyle Smith on 2025-09-26.
//

#include "Engine.h"
#include "engine_GLUT.h"
#include "Colors.h"

namespace glengine {
    Engine::Engine(const std::string &windowName, int2 size) {
        windowSize = size;
        glutInitWindowSize(windowSize.x, windowSize.y);

        windowHandle = glutCreateWindow(windowName.c_str());
    }

    Engine::~Engine() {
        glutDestroyWindow(windowHandle);
    }

    void Engine::Render() {
        clearBuffers();

    }

    void Engine::clearBuffers() {
        glColor4fv(Colors::BLACK);
        glClear(GL_COLOR_BUFFER_BIT);
    }
} // glengine