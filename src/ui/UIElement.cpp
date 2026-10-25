//
// Created by Kyle Smith on 2026-10-24.
//

#include "ui/UIElement.h"

#include <algorithm>

#include "Engine.h"
#include "GLMath.h"

namespace glengine::ui {
    Element::Element() = default;

    float4 Element::Arrange(float4 available) {
        // note: `position` is not applied here; it is only honored by
        // containers that do absolute placement (CanvasView, or elements that
        // read it explicitly like TextBlock under a Button)
        finalRect_ = {
            available.x + margin.x,
            available.y + margin.y,
            size.x > 0 ? size.x : std::max(0.0f, available.z - margin.x - margin.z),
            size.y > 0 ? size.y : std::max(0.0f, available.w - margin.y - margin.w)
        };
        return finalRect_;
    }

    void Element::CollectRenderCommands(std::vector<UIRenderCommand>& commands, const mat3& parentTransform) {
        if (!visible_ || backgroundColor.w <= 0.0f) {
            return;
        }

        const auto local = math::translate2D(float2(finalRect_.x, finalRect_.y)) *
                           math::scale2D(float2(finalRect_.z, finalRect_.w));

        UIRenderCommand command;
        command.type = UIRenderCommand::Type::Rectangle;
        command.zIndex = zIndex;
        command.transform = parentTransform * local;
        command.size = float2(finalRect_.z, finalRect_.w);
        command.color = backgroundColor;
        commands.push_back(command);
    }

    std::shared_ptr<Element> Element::HitTest(float2 point) {
        if (!visible_ || !enabled_ || mouseInteraction == MouseInteraction::Ignore || !ContainsPoint(point)) {
            return nullptr;
        }
        return shared_from_this();
    }

    void Element::Destroy() {
        destroyed_ = true;
    }

    void Element::SetEngine(Engine* engine) {
        const bool wasNull = engine_ == nullptr;
        engine_ = engine;
        if (wasNull && engine != nullptr) {
            OnAddedToEngine();
        }
    }

    float2 Element::ToLocal(float2 point) const {
        return point - float2(finalRect_.x, finalRect_.y);
    }

    bool Element::ContainsPoint(float2 point) const {
        return point.x >= finalRect_.x && point.x <= finalRect_.x + finalRect_.z &&
               point.y >= finalRect_.y && point.y <= finalRect_.y + finalRect_.w;
    }

    View::View() = default;

    void View::AddChild(const std::shared_ptr<Element>& child) {
        if (!child) return;
        child->parent_ = this;
        child->SetEngine(engine_);
        children_.push_back(child);
    }

    void View::RemoveChild(const std::shared_ptr<Element>& child) {
        std::erase(children_, child);
        if (child) {
            child->parent_ = nullptr;
        }
    }

    void View::SetEngine(Engine* engine) {
        Element::SetEngine(engine);
        for (const auto& child : children_) {
            child->SetEngine(engine);
        }
    }

    void View::Update(double deltaTime) {
        Element::Update(deltaTime);
        for (const auto& child : children_) {
            child->Update(deltaTime);
        }
        std::erase_if(children_, [](const std::shared_ptr<Element>& child) { return child->IsDestroyed(); });
    }

    float4 View::Arrange(float4 available) {
        finalRect_ = Element::Arrange(available);

        const float4 content = {
            finalRect_.x + padding.x,
            finalRect_.y + padding.y,
            std::max(0.0f, finalRect_.z - padding.x - padding.z),
            std::max(0.0f, finalRect_.w - padding.y - padding.w)
        };

        for (const auto& child : children_) {
            child->Arrange(content);
        }

        return finalRect_;
    }

    void View::CollectRenderCommands(std::vector<UIRenderCommand>& commands, const mat3& parentTransform) {
        if (!visible_) {
            return;
        }

        Element::CollectRenderCommands(commands, parentTransform);

        // children stack relative to their parent, so a view's zIndex lifts
        // its whole subtree (e.g. the console above all other UI)
        const size_t beforeChildren = commands.size();
        for (const auto& child : children_) {
            child->CollectRenderCommands(commands, parentTransform);
        }
        for (size_t i = beforeChildren; i < commands.size(); i++) {
            commands[i].zIndex += zIndex;
        }
    }

    std::shared_ptr<Element> View::HitTest(float2 point) {
        if (!visible_ || !enabled_ || mouseInteraction == MouseInteraction::Ignore || !ContainsPoint(point)) {
            return nullptr;
        }

        for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
            auto hit = (*it)->HitTest(point);
            if (hit) {
                return hit;
            }
        }

        if (mouseInteraction == MouseInteraction::Ignore) {
            return nullptr;
        }

        return shared_from_this();
    }
}
