//
// Created by Kyle Smith on 2025-10-17.
//
#pragma once
#include "ActorComponent.h"
#include "Transform.h"

namespace glengine::world {
    /// An actor primitive component has a 3d position relative to the owning actor, but are not
    /// rendered on screen. This class can be used for control points or camera positions, for example
    class ActorPrimitiveComponent : public ActorComponent {
    public:
        /// Gets the transform matrix for this component
        mat4 GetTransformMatrix() const {
            return transform_.GetAbsoluteMatrix();
        }

        void SetupAttachment(Transform *parent) {
            transform_.SetParent(parent);
        }

        Transform *GetTransform() {
            return &transform_;
        }

        /// Gets the component's absolute world-space coordinates
        float3 GetAbsolutePosition() const;

        float3 GetForwardVector() const;

    protected:
        Transform transform_;
    };
}
