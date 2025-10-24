//
// Created by Kyle Smith on 2025-10-24.
//
#pragma once
#include <map>

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

        void AddConsoleCommand(const char * name, std::function<void(std::string_view)> func);

    private:
        void updateAutocompleteOptions();
        void execute();

    private:
        std::map<std::string, std::function<void(std::string_view)>> commands;
        std::string _autocompleteOptions;
        std::string _inputBuffer;
        bool _focused = false;
    };
} // glengine::console