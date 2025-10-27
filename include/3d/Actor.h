//
// Created by Kyle Smith on 2025-10-17.
//
#pragma once
#include <vector>
#include <ranges>
#include <memory>

#include "ActorComponent.h"
#include "Transform.h"
#include "glengine_export.h"

namespace glengine {
    class Engine;
}

/// Any object that can be represented in the 3D world.
namespace glengine::world {
    /// Used to store a reference to the current engine, so that
    extern GLENGINE_EXPORT Engine* CURRENT_ENGINE_CONSTRUCTING;

    class GLENGINE_EXPORT Actor : public std::enable_shared_from_this<Actor> {
    public:
        virtual ~Actor() = default;

        virtual void Update(double deltaTime) = 0;

        mat4 GetTransformMatrix() const {
            return transform_.GetMatrix();
        }

        /// Returns a reference to the first component of type T
        template <typename T>
        std::shared_ptr<T> GetComponent() {
            for (const auto &component : components_) {
                if (auto result= std::dynamic_pointer_cast<T>(component)) {
                    return result;
                }
            }
            return nullptr;
        }

        std::span<std::shared_ptr<ActorComponent>> GetComponents() {
            return {components_.begin(), components_.size()};
        }

        Engine *GetEngine() const {
            return engine_ == nullptr ? CURRENT_ENGINE_CONSTRUCTING : engine_;
        }

        void SetEngine(Engine *engine) {
            engine_ = engine;
        }

        void Destroy() {
            destroyed_ = true;
        }

        [[nodiscard]] bool IsDestroyed() const {
            return destroyed_;
        }

        Transform *GetTransform() {
            return &transform_;
        }

    protected:
        template <typename T>
        std::shared_ptr<T> CreateComponent() {
            static_assert(std::is_base_of_v<ActorComponent, T>, "T must derive from ActorComponent");
            auto x = std::make_shared<T>();
            x->SetActor(this);
            components_.push_back(x);
            return x;
        }

    private:
        Transform transform_;
        std::vector<std::shared_ptr<ActorComponent>> components_;
        Engine *engine_= nullptr;

        bool destroyed_ = false;
    };
}

