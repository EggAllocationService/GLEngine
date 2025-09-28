#include "Engine.h"
#include "Engine.h"
//
// Created by Kyle Smith on 2025-09-26.
//

#include "Engine.h"
#include "engine_GLUT.h"
#include "Colors.h"
#include "Widget.h"
#include <map>
#include <iostream>

namespace glengine {
    static std::map<int, Engine*> Instances;

    static void renderExec() {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            Instances[currentWindow]->Render();
        }
    }
    static void updateExec(int) {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            Instances[currentWindow]->Update();
        }
    }

    Engine::Engine(const std::string &windowName, int2 size) {
        windowSize = size;
        glutInitWindowSize(windowSize.x, windowSize.y);

        windowHandle = glutCreateWindow(windowName.c_str());

        Instances[windowHandle] = this;

        glutTimerFunc(1000/maxFPS, updateExec, 0); // 60 fps
        glutDisplayFunc(renderExec);

        setLastUpdate();
    }

    Engine::~Engine() {
        // destroy window
        glutDestroyWindow(windowHandle);

        // remove from global instances table
        auto iter = Instances.find(windowHandle);
        if(iter != Instances.end()) {
            Instances.erase(iter);
        }
    }

    void Engine::Update() {
        double delta = calculateDeltaTime();

        updateWidgets(delta);

        glutPostRedisplay();
        setLastUpdate();

        glutTimerFunc(1000/maxFPS, updateExec, 0);
    }

    void Engine::Render() {
        clearBuffers();

        renderWidgets();

        glFlush();
    }

    double Engine::calculateDeltaTime() {
        auto now = std::chrono::steady_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastUpdate);

        return time.count();
    }

    void Engine::AddOnscreenWidget(Widget* widget)
    {
        widgets.push_back(widget);
    }

    void Engine::clearBuffers() {
        glColor4fv(Colors::BLACK);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Engine::setLastUpdate() {
        lastUpdate = std::chrono::steady_clock::now();
    }

    void Engine::renderWidgets()
    {
        MatrixStack2D stack = MatrixStack2D();

        for (Widget* widget : widgets) {
            stack.Push(widget->GetTransformMatrix());
            widget->Draw(stack);
            stack.Pop();
        }
    }
    void glengine::Engine::updateWidgets(double deltaTime)
    {
        for (Widget* widget : widgets) {
            widget->Update(deltaTime);
        }
    }
} // glengine