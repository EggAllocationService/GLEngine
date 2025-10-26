//
// Created by Kyle Smith on 2025-10-24.
//

#include "console/Console.h"

#include "Colors.h"
#include "Engine.h"
#include "MouseManager.h"

using namespace glengine::console;

Console::Console() {
    _inputBuffer.reserve(128);
    Position = float2(10, 10);
    ZIndex = INT32_MAX;
    MouseMode = MouseInteraction::IGNORED;
}

void Console::Update(double DeltaTime) {
    Bounds = GetEngine()->GetWindowSize() - int2(20, 20);
}

void Console::Draw(MatrixStack2D &stack) {
    if (!_focused) return;

    // draw input background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0, 0.5, 0.0, 0.6);
    stack.DrawRect(float2(0, 0), float2(Bounds.x, 13));

    // text
    glColor4fv(Colors::GREEN);
    // draw line start
    stack.PrintText(float2(0, 1), ">");
    // draw input buffer
    stack.PrintText(float2(10, 1), _inputBuffer.c_str());
    // draw caret
    float offset = _inputBuffer.size() * 8;
    stack.PrintText(float2(offset + 10, 1), "_");

}

void Console::FocusStateChanged(bool focused) {
    _focused = focused;
    if (!focused) {
        _inputBuffer.clear();
    }
}

void Console::KeyPressed(int keyCode) {
    if (keyCode == 127) {
        // backspace
        if (_inputBuffer.size() > 0) {
            _inputBuffer.pop_back();
        }
    } else if (keyCode == 13) {
        // enter
        _inputBuffer.clear();
    } else if (keyCode >= 32 && keyCode <= 126) {
        _inputBuffer.push_back(keyCode);
    } else if (keyCode == 27) {
        // escape
        GetEngine()->FocusWidget(nullptr);
    }
}
