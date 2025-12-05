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
        } else {
            auto windowMiddle = float2(engine->GetWindowSize() / 2);
            auto delta = position - windowMiddle;
            mousePosition = delta;
            centerCursor();
        }
    }

    void MouseManager::Click(int button, int action, float2 pos) {
        // need to update this even if we're not clicking any widget, so we don't send drag/mouseup events to a widget
        // before a mousedown event
        clicking = action == GLUT_DOWN;
    }


    void MouseManager::SetMouseMode(MouseMode mode) {
        mouseMode = mode;
        if (mode == FREE) {
            // do thing
        } else {
            // hide the cursor and center it
            centerCursor();

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
