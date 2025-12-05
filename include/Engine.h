//
// Created by Kyle Smith on 2025-09-26.
//
#pragma once

#include <string>
#include <vector>
#include <ranges>
#include <chrono>
#include "Vectors.h"
#include "MouseManager.h"
#include "3d/Actor.h"
#include "3d/Pawn.h"
#include "InputManager.h"
#include "glengine_export.h"
#include "ResourceManager.h"
#include "pipeline/RenderObjects.h"
#include "GLFW/glfw3.h"
#include "pipeline/Renderer.h"

namespace glengine {
    struct EnginePerformanceStats { double update, render; };

    class GLENGINE_EXPORT Engine {
    public:
        Engine(const std::string &windowName, int2 windowSize);

        ~Engine();

        void MainLoop();

        /// Renders all renderable objects to the screen
        void Render();

        /// Simulates the game world
        void Update();

        void Quit() {
            flags.quitRequested = true;
        }

        /// Call when a key has been pressed
        void KeyPressed(int keyCode);

        /// Call when a key has been released
        void KeyReleased(int keyCode);

        /// Sets the current mouse mode to `mode`
        void SetMouseMode(input::MouseMode mode);

        template <typename T>
        std::shared_ptr<T> SpawnActor() {
            static_assert(std::is_base_of_v<world::Actor, T>, "T must be derived from Actor");
            // set global engine reference
            world::CURRENT_ENGINE_CONSTRUCTING = this;

            std::shared_ptr<T> actor = std::make_shared<T>();
            actor->SetEngine(this);
            actors.push_back(actor);
            return actor;
        }

        void Possess(const std::shared_ptr<world::Pawn>& target);

        /// Helper function to find all actors of a given type
        template <typename T>
        auto GetActorsOfType() {
            static_assert(std::is_base_of_v<world::Actor, T>, "T must be derive from Actor");

            return std::ranges::views::transform(actors, [](std::shared_ptr<world::Actor> actor) {return dynamic_pointer_cast<T>(actor); })
                | std::ranges::views::filter([](std::shared_ptr<T> transformed) {return transformed != nullptr; });
        }

#pragma region Getters/Setters

        void SetAllowNonFocusedPawnInput(bool value) {
            flags.allowNonFocusedPawnInput = value;
        }

        int2 GetWindowSize() const {
            return windowSize;
        }

        /// Call to update internal states whenever the window size changes
        void SetWindowSize(int2 size);

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

        [[nodiscard]] rendering::RenderObjects* GetRenderObjectsManager() const {
            return renderObjectManager;
        }

        [[nodiscard]] world::Pawn* GetPossessedPawn() const {
            if (possessedPawn.expired()) {
                return nullptr;
            }

            return possessedPawn.lock().get();
        }

        /// <summary>
        /// Returns the time (in milliseconds) the previous Update and Render functions took to run
        /// </summary>
        [[nodiscard]] EnginePerformanceStats GetLastPerformanceTimes() {
            return { lastUpdateTime, lastRenderTime };
        }
#pragma endregion

    private:
        int2 windowSize;
        GLFWwindow* window;

        // used for tracking deltaTime
        std::chrono::steady_clock::time_point lastUpdate;

        /// <summary>
        /// Gets the number of seconds since the last Update() call.
        /// This will almost always be a very small number
        /// </summary>
        [[nodiscard]] double calculateDeltaTime() const;

        void clearBuffers();

        void setLastUpdate();

        void updateActors(double deltaTime);

        void renderWorld() const;

        input::MouseManager* mouseManager;

        // Two input managers are needed, one for global keybinds and one for the currently possessed pawn
        input::InputManager* inputManager;
        input::InputManager* pawnInputManager;

        ResourceManager* resourceManager;

        rendering::RenderObjects* renderObjectManager;
        Renderer* renderer;

        std::vector<std::shared_ptr<world::Actor>> actors;

        std::weak_ptr<world::Pawn> possessedPawn;

        double lastUpdateTime = 0.0;
        double lastRenderTime = 0.0;

        struct {
            /// When true, causes the engine to destroy the window and exit on the next frame.
            bool quitRequested = false;

            /// If true, keyboard input is directed to the possessed pawn even if the mouse mode is not `CAPTIVE`
            bool allowNonFocusedPawnInput = false;

            /// Makes sure we don't Render() before Update() has been called at least once
            bool didUpdate = false;
        } flags;
    };
} // glengine