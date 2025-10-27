//
// Created by Kyle Smith on 2025-09-26.
//
#pragma once

#include <string>
#include <vector>
#include <ranges>
#include <chrono>
#include "Vectors.h"
#include "Widget.h"
#include "MouseManager.h"
#include "3d/Actor.h"
#include "3d/Pawn.h"
#include "InputManager.h"
#include "glengine_export.h"
#include "ResourceManager.h"
#include "console/Console.h"

namespace glengine {
    class GLENGINE_EXPORT Engine {
    public:
        Engine(const std::string &windowName, int2 windowSize);

        ~Engine();

        /// Renders all renderable objects to the screen
        void Render();

        /// Simulates the game world
        void Update();

        void Quit() {
            quitRequested = true;
        }

        /// Call when a key has been pressed
        void KeyPressed(int keyCode);

        /// Call when a key has been released
        void KeyReleased(int keyCode);

        /// call to focus this widget
        /// only works when mouse mode is FREE
        void FocusWidget(std::shared_ptr<Widget> widget);

        /// Sets the current mouse mode to `mode`
        void SetMouseMode(input::MouseMode mode);

        void ShowConsole() {
            FocusWidget(console);
        }

        /// Creates a new widget of type T and adds it to the viewport
        /// Returns a shared reference to the created widget
        template <typename T>
        std::shared_ptr<T> AddOnscreenWidget() {
            static_assert(std::is_base_of_v<Widget, T>, "T must be derived from Widget");

            std::shared_ptr<T> widget = Widget::New<T>(this);
            widgets.push_back(widget);
            return widget;
        }

        template <typename T>
        std::shared_ptr<T> SpawnActor() {
            static_assert(std::is_base_of_v<world::Actor, T>, "T must be derived from Actor");
            std::shared_ptr<T> actor = std::make_shared<T>();
            actor->SetEngine(this);
            actors.push_back(actor);
            return actor;
        }

        void Possess(const std::shared_ptr<world::Pawn>& target);

        /// Helper function to find all widgets of a given type
        template <typename T>
        auto GetWidgetsOfType() {
            static_assert(std::is_base_of_v<Widget, T>, "T must be derive from Widget");

            return std::ranges::views::transform(widgets, [](std::shared_ptr<Widget> widget) {return dynamic_pointer_cast<T>(widget); })
                | std::ranges::views::filter([](std::shared_ptr<T> transformed) {return transformed != nullptr; });
        }

        int2 GetWindowSize() const {
            return windowSize;
        }

        /// Call to update internal states whenever the window size changes
        void SetWindowSize(int2 size);

        /// Get a reference to the list of active widgets
        std::vector<std::shared_ptr<Widget>>& GetWidgets() {
            return widgets;
        }

        [[nodiscard]] input::MouseManager* GetMouseManager() const {
            return mouseManager;
        }

        [[nodiscard]] input::InputManager* GetInputManager() const {
            return inputManager;
        }

        [[nodiscard]] input::InputManager* GetPawnInputManager() const {
            return pawnInputManager;
        }

        [[nodiscard]] ResourceManager* GetResourceManager() const {
            return resourceManager;
        }

        [[nodiscard]] world::Pawn* GetPossessedPawn() const {
            if (possessedPawn.expired()) {
                return nullptr;
            }

            return possessedPawn.lock().get();
        }

        [[nodiscard]] std::shared_ptr<console::Console> GetConsole() const {
            return console;
        }

    private:
        int2 windowSize;
        int windowHandle;

        // used for tracking deltaTime
        std::chrono::steady_clock::time_point lastUpdate;

        /// When true, causes the engine to destroy the window and exit on the next frame.
        bool quitRequested = false;

        /// <summary>
        /// Gets the number of seconds since the last Update() call.
        /// This will almost always be a very small number
        /// </summary>
        [[nodiscard]] double calculateDeltaTime() const;

        void clearBuffers();

        void setLastUpdate();

        void renderWidgets();

        void updateWidgets(double deltaTime);

        void updateActors(double deltaTime);

        void renderWorld() const;

        void addDefaultCommands();

        input::MouseManager* mouseManager;

        // Two input managers are needed, one for global keybinds and one for the currently possessed pawn
        input::InputManager* inputManager;
        input::InputManager* pawnInputManager;

        ResourceManager* resourceManager;

        std::vector<std::shared_ptr<Widget>> widgets;
        std::vector<std::shared_ptr<world::Actor>> actors;

        std::weak_ptr<world::Pawn> possessedPawn;
        std::weak_ptr<Widget> focusedWidget;
        std::shared_ptr<console::Console> console;
    };
} // glengine