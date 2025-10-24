//
// Created by Kyle Smith on 2025-10-24.
//
#pragma once
#include "Widget.h"
#include <string>

namespace glengine::console {
    class Console : public Widget {
    public:
        Console();
        void Update(double DeltaTime) override;
        void Draw(MatrixStack2D &stack) override;
        void FocusStateChanged(bool focused) override;
        void KeyPressed(int keyCode) override;

    private:
        std::string _inputBuffer;
        bool _focused = false;
    };
} // glengine::console