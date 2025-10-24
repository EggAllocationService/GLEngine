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
    _autocompleteOptions.reserve(128);

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
    float caretOffset = (_inputBuffer.size() * 8) + 10;
    stack.PrintText(float2(caretOffset , 1), "_");

    // draw suggestions
    if (!_autocompleteOptions.empty()) {
        // find count
        int count = std::ranges::count(_autocompleteOptions, '\n');
        int longestSuggestion = 0;
        for (auto suggestion : std::ranges::views::split(_autocompleteOptions, '\n')) {
            longestSuggestion = std::max(longestSuggestion, (int)suggestion.size());
        }

        float2 bottomLeft = float2(caretOffset, 26);
        float2 topRight = float2(caretOffset + (longestSuggestion * 8), 13 * (count + 2));
        float2 topLeft = float2(caretOffset, 13 * (count + 1));
        glColor4f(0.0, 0.5, 0.0, 0.6);
        stack.DrawRect(bottomLeft, topRight);

        glColor4fv(Colors::GREEN);
        stack.PrintText(topLeft, _autocompleteOptions.c_str());
    }

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
        updateAutocompleteOptions();
    } else if (keyCode == 13) {
        // enter
        execute();

        _inputBuffer.clear();
        _autocompleteOptions.clear();
    } else if (keyCode >= 32 && keyCode <= 126) {
        _inputBuffer.push_back(keyCode);
        updateAutocompleteOptions();
    } else if (keyCode == 27) {
        // escape
        GetEngine()->FocusWidget(nullptr);
    }
}

void Console::AddConsoleCommand(const char *name, std::function<void(std::string_view)> func) {
    std::string ownedString(name);
    if (ownedString.find(' ') != std::string::npos) {
        return;
    }

    commands[ownedString] = func;
}

void Console::updateAutocompleteOptions() {
    _autocompleteOptions.clear();
    if (_inputBuffer.size() == 0) return;
    // don't autocomplete anything other than the command name itself
    // which will be before the first space
    if (_inputBuffer.find_first_of(' ') != std::string::npos) return;

    int found = 0;
    for (const auto& cmd : commands) {
        auto& name = cmd.first;

        if (name.starts_with(_inputBuffer)) {
            _autocompleteOptions += name;
            _autocompleteOptions.push_back('\n');
            found++;
        }

        if (found == 5) {
            // only offer 5 autocomplete suggestions
            break;
        }
    }
}

void Console::execute() {
    if (_inputBuffer.empty()) return;

    auto firstSpace = _inputBuffer.find_first_of(' ');
    if (firstSpace == std::string::npos) {
        const auto cmd = commands.find(_inputBuffer);
        if (cmd != commands.end()) {
            cmd->second(std::string_view(nullptr, 0));
        }
    } else {
        auto commandName = _inputBuffer.substr(0, firstSpace);
        const auto cmd = commands.find(commandName);
        if (cmd != commands.end()) {
            cmd->second(std::string_view(_inputBuffer.data() + firstSpace, _inputBuffer.size() - firstSpace));
        }
    }
}
