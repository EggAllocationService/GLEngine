//
// Created by Kyle Smith on 2025-10-01.
//

#include "MouseManager.h"
#include "Engine.h"
#include "Widget.h"
#include "engine_GLUT.h"
#include <ranges>

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
        // since we're hit-testing on every frame, we should already know what's being clicked
        auto target = hoveredWidget.lock();

        if (target != nullptr) {
            target->Click(button, action, pos - target->GetEffectiveAbsolutePosition());
        }

        // need to update this even if we're not clicking any widget, so we don't send drag/mouseup events to a widget
        // before a mousedown event
        clicking = action == GLUT_DOWN;
    }

    void MouseManager::Update() {
        // every frame in free mouse mode, hit test under the cursor
        // this makes sure moving widgets get hover events correctly
        // but if the user is clicking, we don't want to change the hovered widget
        // because then buttons may not get the mouse up event, for example
        if (mouseMode == FREE) {
            // check if we're still dragging from the same element
            if (clicking) {
                if (hoveredWidget.expired()) {
                    // if the object we were clicking has been freed, then set clicking to false
                    clicking = false;
                }
                else {
                    // if it's not freed, then send drag events
                    auto hovered = hoveredWidget.lock();
                    hovered->Click(GLUT_LEFT_BUTTON, GLUT_DRAG, mousePosition);
                    return;
                }
            }

            // since the user isn't clicking we need to update whatever's being hovered
            auto newHovered = hitTestWidgets(mousePosition);
            if (newHovered == hoveredWidget.lock()) return; // still hovering the same widget

            if (!hoveredWidget.expired()) {
                // previously hovered widget is no longer hovered
                hoveredWidget.lock()->HoverStateChanged(false);
            }

            if (newHovered != nullptr) {
                // new widget hovered
                newHovered->HoverStateChanged(true);
                glutSetCursor(newHovered->Cursor);
            }
            else {
                // was hovering something, now hovering nothing
                glutSetCursor(GLUT_CURSOR_INHERIT);
            }

            hoveredWidget = newHovered;
        }
    }

    void MouseManager::SetMouseMode(MouseMode mode) {
        // TODO: update this when implementing 3d control
        // not needed at the moment
        mouseMode = mode;
        if (mode == FREE) {
            glutSetCursor(GLUT_CURSOR_INHERIT);
        } else {
            glutSetCursor(GLUT_CURSOR_NONE);
            centerCursor();
            hoveredWidget = std::weak_ptr<Widget>();
            mousePosition = float2(0, 0);
        }
    }

    void MouseManager::centerCursor() const {
        // teleport the cursor to the center of the window
        int2 windowMiddle = engine->GetWindowSize() / 2;
        glutWarpPointer(windowMiddle.x, windowMiddle.y);
    }

    inline bool internal_isPointInside(float2 point, float2 a, float2 b) {
        return a < point && point < b;
    }

    /// Looks in a widgets children recursively to find out what is hovered
    /// <param name="transform">The current transform. This should be the on-screen position of `widget`</param>
    /// <param name="widget"> The widget to hit test, and check the children</param>
    /// <param name="position">Window-space coordinates of the mouse cursor</param>
    std::shared_ptr<Widget> internal_hitTestWidgetAndChildren(float2 transform, std::shared_ptr<Widget> widget,
                                                              float2 position) {

        if (widget->MouseMode == IGNORE) {
            return nullptr;
        }

        float2 lowerLeft = transform;
        float2 topRight = transform + widget->Bounds;

        if (widget->GetChildren().empty()) {
            if (internal_isPointInside(position, lowerLeft, topRight)) {
                return widget;
            }
        } else {
            // children are sorted in ascending z-order each frame, so iterate high -> low
            for (const auto& child: std::ranges::views::reverse(widget->GetChildren())) {
                auto child_result = internal_hitTestWidgetAndChildren(transform + child->GetEffectiveRelativePosition(),
                                                                      child, position);

                if (child_result != nullptr) return child_result;
            }

            // no children passed the hit test, see if the widget itself still matches
            if (internal_isPointInside(position, transform, transform + widget->Bounds)) {
                return widget;
            }
        }

        return nullptr;
    }

    std::shared_ptr<Widget> MouseManager::hitTestWidgets(const float2 position) const {
        auto widgets = engine->GetWidgets();

        // widgets array is sorted by ascending z-index every tick, so search in reverse order
        for (const auto& widget: std::ranges::views::reverse(widgets)) {
            auto result = internal_hitTestWidgetAndChildren(widget->GetEffectiveRelativePosition(), widget, position);

            if (result != nullptr) return result;
        }

        // nothing hovered
        return nullptr;
    }
}
