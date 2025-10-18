//
// Created by Kyle Smith on 2025-10-17.
//
#pragma once
#include <vector>

#include "ActorComponent.h"
#include "Engine.h"
#include "Transform.h"

/// Any object that can be represented in the 3D world.
namespace glengine::world {
    class Actor {
    public:
        virtual ~Actor() = default;

        virtual void Update(double deltaTime) = 0;

        mat4 GetTransformMatrix() const {
            return static_cast<mat4>(transform_);
        }

        /// Returns a reference to the first component of type T
        template <typename T>
        std::weak_ptr<T> GetComponent() {
            for (const auto &component : components_) {
                if (auto result= std::dynamic_pointer_cast<T>(component)) {
                    return result;
                }
            }
            return std::weak_ptr<T>();
        }

        Engine *GetEngine() {
            return engine_;
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

    protected:
        template <typename T>
        std::weak_ptr<T> CreateComponent() {
            static_assert(std::is_base_of<ActorComponent, T>::value, "T must derive from ActorComponent");
            auto x = std::make_shared<T>();
            x->SetActor(this);
            components_.push_back(x);
            return x;
        }

    private:
        Transform transform_;
        std::vector<std::shared_ptr<ActorComponent>> components_;
        Engine *engine_;

        bool destroyed_ = false;
    };
}

