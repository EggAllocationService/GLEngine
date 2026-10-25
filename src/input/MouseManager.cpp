//
// Created by Kyle Smith on 2025-10-01.
//

#include "MouseManager.h"
#include "Engine.h"

namespace glengine::input {
    MouseManager::MouseManager(Engine *engine) {
        this->engine = engine;
    }

    void MouseManager::HandleMotion(float2 position) {
        if (mouseMode == FREE) {
            mousePosition = position;
#ifdef GLENGINE_TEXT_RENDERING
            // convert to bottom-left origin for the UI
            engine->GetUIManager()->HandleMouseMove(float2(position.x, engine->GetWindowSize().y + position.y));
#endif
        } else {
            auto delta = position - mousePosition;
            mousePosition = position;
            engine->GetPawnInputManager()->AcceptMouseInput(delta);
            engine->GetInputManager()->AcceptMouseInput(delta);
        }
    }

    void MouseManager::Click(int button, int action, float2 pos) {
        clicking = action == GLFW_PRESS;
#ifdef GLENGINE_TEXT_RENDERING
        if (mouseMode == FREE) {
            // convert to bottom-left origin for the UI
            engine->GetUIManager()->HandleMouseClick(button, action == GLFW_PRESS,
                float2(pos.x, engine->GetWindowSize().y + pos.y));
        }
#endif
    }


    void MouseManager::SetMouseMode(MouseMode mode) {
        mouseMode = mode;
        if (mode == FREE) {
            // do thing
            glfwSetInputMode(engine->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(engine->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // reset state
            mousePosition = float2(0, 0);
        }
    }

    void MouseManager::centerCursor() const {
        // teleport the cursor to the center of the window
        int2 windowMiddle = engine->GetWindowSize() / 2;
        // TODO: implement proper captive cursor
    }
}
