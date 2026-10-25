//
// Created by Kyle Smith on 2026-10-25.
//
#pragma once

#include <deque>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "ui/UIElement.h"
#include "glengine_export.h"

namespace glengine::ui {
    class TextBlock;

    /// Interactive console with an input bar, autocomplete suggestions, and a
    /// log message box above the input. Toggle with Engine::ShowConsole()
    /// (bound to backtick by default). Write to the log with Engine::LogConsole.
    class GLENGINE_EXPORT Console : public View {
    public:
        Console();

        void OnAddedToEngine() override;
        void Update(double deltaTime) override;
        float4 Arrange(float4 available) override;
        void CollectRenderCommands(std::vector<UIRenderCommand>& commands, const mat3& parentTransform) override;
        std::shared_ptr<Element> HitTest(float2 point) override;

        void OnFocusChanged(bool focused) override;
        void OnKeyPressed(int keyCode) override;

        void AddConsoleCommand(const char* name, std::function<void(std::string_view)> func);

        /// Appends a message to the log box. Multi-line messages are split.
        void Log(std::string_view message);

        bool IsOpen() const { return open_; }
        void SetOpen(bool open);

    private:
        void RefreshLogLines();
        void RefreshInputLine();
        void Execute();

        std::map<std::string, std::function<void(std::string_view)>> commands_;
        std::string inputBuffer_;
        std::string shownInput_;
        std::deque<std::string> logLines_;
        bool logDirty_ = true;
        bool open_ = false;

        std::shared_ptr<View> logBox_;
        std::vector<std::shared_ptr<TextBlock>> logTextBlocks_;
        std::shared_ptr<View> inputBar_;
        std::shared_ptr<TextBlock> inputText_;
    };
}
