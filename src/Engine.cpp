#include "Engine.h"
//
// Created by Kyle Smith on 2025-09-26.
//
#include "engine_GLUT.h"
#include "Colors.h"
#include <map>

namespace glengine {
    static std::map<int, Engine *> Instances;

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

    static void reshapeExec(int x, int y) {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            Instances[currentWindow]->SetWindowSize(int2(x, y));
        }
    }

    Engine::Engine(const std::string &windowName, int2 size) {
        windowSize = size;
        glutInitWindowSize(windowSize.x, windowSize.y);

        windowHandle = glutCreateWindow(windowName.c_str());

        Instances[windowHandle] = this;

        glutTimerFunc(1000 / maxFPS, updateExec, 0); // 60 fps
        glutDisplayFunc(renderExec);
        glutReshapeFunc(reshapeExec);

        setLastUpdate();
    }

    Engine::~Engine() {
        // destroy window
        glutDestroyWindow(windowHandle);

        // remove from global instances table
        auto iter = Instances.find(windowHandle);
        if (iter != Instances.end()) {
            Instances.erase(iter);
        }
    }

    void Engine::Update() {
        double delta = calculateDeltaTime();

        updateWidgets(delta);

        glutPostRedisplay();
        setLastUpdate();

        glutTimerFunc(1000 / maxFPS, updateExec, 0);
    }

    void Engine::SetWindowSize(int2 size) {
        windowSize = size;
        glViewport(0, 0, windowSize.x, windowSize.y);
    }

    void Engine::Render() {
        clearBuffers();

        renderWidgets();

        glFlush();
    }

    double Engine::calculateDeltaTime() {
        auto now = std::chrono::steady_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::duration<double> >(now - lastUpdate);

        return time.count();
    }

    void Engine::clearBuffers() {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Engine::setLastUpdate() {
        lastUpdate = std::chrono::steady_clock::now();
    }

    void Engine::renderWidgets() {
        MatrixStack2D stack = MatrixStack2D();

        // generate screen pixel coord -> gl coord matrix
        mat3 screenTransform = mat3::identity();
        // set scales
        screenTransform[0]->set(0, 2.0f / windowSize.x);
        screenTransform[1]->set(1, 2.0f / windowSize.y);
        screenTransform[2]->set(0, -1.0f);
        screenTransform[2]->set(1, -1.0f);

        stack.Push(screenTransform);

        for (auto widget: widgets) {
            stack.Push(widget->GetTransformMatrix());
            widget->Draw(stack);
            stack.Pop();
        }
    }

    void Engine::updateWidgets(double deltaTime) {
        for (auto widget: widgets) {
            widget->UpdateAll(deltaTime);
        }
    }
} // glengine
