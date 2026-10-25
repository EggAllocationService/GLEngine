//
// Created by Kyle Smith on 2026-10-25.
//

#include "ui/Console.h"

#include <algorithm>
#include <cstdint>
#include <ranges>

#include "Engine.h"
#include "ui/TextBlock.h"

namespace glengine::ui {
    namespace {
        constexpr float MARGIN = 10.0f;
        constexpr float INPUT_HEIGHT = 26.0f;
        constexpr float GAP = 6.0f;
        constexpr float LINE_HEIGHT = 18.0f;
        constexpr float TEXT_SIZE = 15.0f;
        constexpr int MAX_LOG_BLOCKS = 64;
        constexpr int LOG_CAP = 100;

        const float4 BAR_COLOR = float4(0.0f, 0.0f, 0.0f, 1.0f);
        const float4 LOG_COLOR = float4(0.0f, 0.0f, 0.0f, 1.0f);
        const float4 TEXT_COLOR = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    Console::Console() {
        margin = float4(MARGIN, MARGIN, MARGIN, MARGIN);
        zIndex = INT32_MAX;
        mouseInteraction = MouseInteraction::Ignore;
        // the console is keyboard-driven; it needs focus to receive input
        focusable = true;

        logBox_ = AddElement<View>();
        logBox_->margin = float4(0, 8, 0, 0);
        logBox_->backgroundColor = LOG_COLOR;
        logBox_->mouseInteraction = MouseInteraction::Ignore;
        logTextBlocks_.reserve(MAX_LOG_BLOCKS);
        for (int i = 0; i < MAX_LOG_BLOCKS; i++) {
            auto line = logBox_->AddElement<TextBlock>();
            line->SetFontSize(TEXT_SIZE);
            line->foregroundColor = TEXT_COLOR;
            line->position = float2(8, 3);
            logTextBlocks_.push_back(line);
        }

        inputBar_ = AddElement<View>();
        inputBar_->backgroundColor = BAR_COLOR;
        inputBar_->mouseInteraction = MouseInteraction::Ignore;
        inputText_ = inputBar_->AddElement<TextBlock>();
        inputText_->SetFontSize(TEXT_SIZE);
        inputText_->foregroundColor = TEXT_COLOR;
        // center vertically using the nominal font size; measuring the actual
        // text here would make the offset shift as the content changes
        inputText_->position = float2(8, (INPUT_HEIGHT - TEXT_SIZE) * 0.5f);
    }

    void Console::OnAddedToEngine() {
        auto font = engine_->GetResourceManager()->GetResource<world::font::Font>("/builtin/fonts/trim.ttf");
        inputText_->SetFont(font);
        for (const auto& line : logTextBlocks_) line->SetFont(font);
    }

    void Console::SetOpen(bool open) {
        open_ = open;
        if (!open) {
            inputBuffer_.clear();
        }
    }

    void Console::OnFocusChanged(bool focused) {
        if (focused) {
            open_ = true;
        } else {
            SetOpen(false);
        }
    }

    float4 Console::Arrange(float4 available) {
        finalRect_ = Element::Arrange(available);
        if (!open_) return finalRect_;

        const float x = finalRect_.x, y = finalRect_.y, w = finalRect_.z, h = finalRect_.w;

        inputBar_->Arrange({x, y, w, INPUT_HEIGHT});
        inputText_->Arrange(inputBar_->GetFinalRect());

        const float logY = y + INPUT_HEIGHT + GAP;
        const float logH = h - INPUT_HEIGHT - GAP;
        logBox_->Arrange({x, logY, w, logH});

        const int lineCapacity = std::min(MAX_LOG_BLOCKS, (int)(logH / LINE_HEIGHT));
        for (int i = 0; i < MAX_LOG_BLOCKS; i++) {
            const bool used = i < lineCapacity;
            logTextBlocks_[i]->SetVisible(used);
            if (used) {
                const float lineY = logY + logH - (i + 1) * LINE_HEIGHT;
                logTextBlocks_[i]->Arrange({x, lineY, w, LINE_HEIGHT});
            }
        }

        return finalRect_;
    }

    void Console::Update(double deltaTime) {
        if (open_) {
            RefreshInputLine();
            if (logDirty_) {
                RefreshLogLines();
                logDirty_ = false;
            }
        }
        View::Update(deltaTime);
    }

    void Console::RefreshInputLine() {
        const std::string current = "> " + inputBuffer_ + "_";
        if (current != shownInput_) {
            shownInput_ = current;
            inputText_->SetText(shownInput_);
        }
    }

    void Console::RefreshLogLines() {
        // most recent messages at the bottom
        const size_t count = logTextBlocks_.size();
        const size_t start = logLines_.size() > count ? logLines_.size() - count : 0;
        for (size_t i = 0; i < count; i++) {
            if (start + i < logLines_.size()) {
                logTextBlocks_[i]->SetText(logLines_[start + i]);
            } else {
                logTextBlocks_[i]->SetText("");
            }
        }
    }

    void Console::CollectRenderCommands(std::vector<UIRenderCommand>& commands, const mat3& parentTransform) {
        if (!open_) return;
        View::CollectRenderCommands(commands, parentTransform);
    }

    std::shared_ptr<Element> Console::HitTest(float2 point) {
        if (!open_) return nullptr;
        return View::HitTest(point);
    }

    void Console::OnKeyPressed(int keyCode) {
        if (keyCode == GLFW_KEY_BACKSPACE) {
            if (!inputBuffer_.empty()) {
                inputBuffer_.pop_back();
            }
        } else if (keyCode == GLFW_KEY_ENTER) {
            Execute();
            inputBuffer_.clear();
        } else if (keyCode >= 32 && keyCode <= 126) {
            char c = (char)keyCode;
            // GLFW key codes for letters are always uppercase
            if (c >= 'A' && c <= 'Z') {
                c += 'a' - 'A';
            }
            inputBuffer_.push_back(c);
        } else if (keyCode == GLFW_KEY_ESCAPE) {
            engine_->HideConsole();
        }
    }

    void Console::AddConsoleCommand(const char* name, std::function<void(std::string_view)> func) {
        std::string ownedString(name);
        if (ownedString.find(' ') != std::string::npos) {
            return;
        }
        commands_[ownedString] = std::move(func);
    }

    void Console::Execute() {
        if (inputBuffer_.empty()) return;

        const auto firstSpace = inputBuffer_.find_first_of(' ');
        const auto commandName = inputBuffer_.substr(0, firstSpace);
        const auto cmd = commands_.find(commandName);
        if (cmd != commands_.end()) {
            const auto args = firstSpace == std::string::npos
                ? std::string_view(nullptr, 0)
                : std::string_view(inputBuffer_.data() + firstSpace + 1, inputBuffer_.size() - (firstSpace + 1));
            cmd->second(args);
            engine_->HideConsole();
        }
    }

    void Console::Log(std::string_view message) {
        size_t begin = 0;
        while (begin <= message.size()) {
            const size_t end = message.find('\n', begin);
            logLines_.emplace_back(message.substr(begin, end == std::string_view::npos ? end : end - begin));
            if (logLines_.size() > LOG_CAP) {
                logLines_.pop_front();
            }
            if (end == std::string_view::npos) break;
            begin = end + 1;
        }
        logDirty_ = true;
    }
}
