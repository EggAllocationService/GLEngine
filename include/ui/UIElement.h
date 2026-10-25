//
// Created by Kyle Smith on 2026-10-24.
//
#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <string>

#include "Vectors.h"
#include "Matrix.h"
#include "glengine_export.h"

namespace glengine {
    class Engine;
}

namespace glengine::pipeline::wgpu {
    class GPUMesh;
}

namespace glengine::world::font {
    class Font;
}

namespace glengine::ui {
    class Element;
    class View;

    enum class GLENGINE_EXPORT Alignment {
        Start,
        Center,
        End,
        Stretch
    };

    enum class GLENGINE_EXPORT MouseInteraction {
        Block,
        Ignore
    };

    /// System cursor shown while hovering an element.
    enum class GLENGINE_EXPORT Cursor {
        Default,
        Pointer,
        Text,
        Crosshair,
        ResizeHorizontal,
        ResizeVertical,
        ResizeDiagonalRight,
        ResizeDiagonalLeft,
        ResizeAll,
        NotAllowed
    };

    /// A single UI render command produced by the UI tree.
    /// The UIManager collects these, sorts them, and issues draw calls.
    struct GLENGINE_EXPORT UIRenderCommand {
        enum class Type {
            Rectangle,
            Text,
        };

        Type type;
        int zIndex = 0;
        mat3 transform;     // screen-pixel transform (bottom-left origin)
        float2 size;        // pixel size
        float4 color;

        // For text commands
        std::shared_ptr<class pipeline::wgpu::GPUMesh> textMesh;
        std::shared_ptr<class world::font::Font> font;
    };

    /// Base class for every node in the UI tree.
    class GLENGINE_EXPORT Element : public std::enable_shared_from_this<Element> {
    public:
        Element();
        virtual ~Element() = default;

        /// Called once per frame. Override for animation/state changes.
        virtual void Update(double deltaTime) {}

        /// Arranges this element and its children. `available` is the pixel-space
        /// rectangle this element may fill, in bottom-left origin.
        /// Returns the element's final bounding box.
        virtual float4 Arrange(float4 available);

        /// Generates render commands for this element and its children.
        /// `commands` is appended to. `parentTransform` is the accumulated 2D transform.
        virtual void CollectRenderCommands(std::vector<UIRenderCommand>& commands, const mat3& parentTransform);

        /// Hit-tests this element in pixel-space coordinates (bottom-left origin).
        /// Returns the deepest leaf element at `point`, or nullptr.
        virtual std::shared_ptr<Element> HitTest(float2 point);

        /// Elements with mouseInteraction == Ignore are skipped by hit-testing.

        /// Input callbacks
        virtual void OnMouseEnter() {}
        virtual void OnMouseLeave() {}
        virtual void OnMouseDown(int button, float2 localPos) {}
        virtual void OnMouseUp(int button, float2 localPos) {}
        virtual void OnMouseDrag(float2 localPos) {}
        virtual void OnKeyPressed(int keyCode) {}
        virtual void OnFocusChanged(bool focused) {}

        /// Mark this element for destruction on the next update.
        void Destroy();
        bool IsDestroyed() const { return destroyed_; }

        /// Visibility
        void SetVisible(bool visible) { visible_ = visible; }
        bool IsVisible() const { return visible_; }

        void SetEnabled(bool enabled) { enabled_ = enabled; }
        bool IsEnabled() const { return enabled_; }

        /// Layout properties (pixels, bottom-left origin)
        float2 position = float2(0, 0);
        float2 size = float2(0, 0);
        float4 margin = float4(0, 0, 0, 0);   // left, top, right, bottom
        float4 padding = float4(0, 0, 0, 0);  // left, top, right, bottom

        /// Visual
        float4 backgroundColor = float4(0, 0, 0, 0);
        float4 foregroundColor = float4(1, 1, 1, 1);

        /// Stacking order. Higher values draw on top.
        int zIndex = 0;

        MouseInteraction mouseInteraction = MouseInteraction::Block;

        /// System cursor displayed while the mouse hovers this element.
        Cursor cursor = Cursor::Default;

        /// Whether this element can receive keyboard focus (e.g. by being clicked).
        /// Focused elements consume keyboard input before it reaches the pawn.
        bool focusable = false;

        View* GetParent() const { return parent_; }
        Engine* GetEngine() const { return engine_; }

        /// Sets the engine for this element and, for views, the whole subtree.
        /// Fires OnAddedToEngine the first time a non-null engine is assigned,
        /// which is the place to fetch engine resources (fonts, etc).
        virtual void SetEngine(Engine* engine);

        /// Called once when the element becomes part of an engine's UI tree.
        virtual void OnAddedToEngine() {}

        /// Final pixel-space rectangle after Arrange(), in bottom-left origin.
        float4 GetFinalRect() const { return finalRect_; }

        /// Convert local pixel-space point to element-local coordinates.
        float2 ToLocal(float2 point) const;

        /// Tests whether a pixel-space point is inside this element's final rect.
        bool ContainsPoint(float2 point) const;

        friend class View;
        friend class UIManager;

    protected:
        Engine* engine_ = nullptr;
        View* parent_ = nullptr;
        float4 finalRect_ = float4(0, 0, 0, 0);
        bool destroyed_ = false;
        bool visible_ = true;
        bool enabled_ = true;

    private:
    };

    /// A View is an Element that can contain children.
    class GLENGINE_EXPORT View : public Element {
    public:
        View();
        ~View() override = default;

        /// Creates and adds a child element of type T. Safe to call from a
        /// view's constructor, like Actor::CreateComponent.
        template <typename T, typename... Args>
        std::shared_ptr<T> AddElement(Args&&... args) {
            static_assert(std::is_base_of_v<Element, T>, "T must derive from Element");
            auto child = std::make_shared<T>(std::forward<Args>(args)...);
            AddChild(child);
            return child;
        }

        void AddChild(const std::shared_ptr<Element>& child);
        void RemoveChild(const std::shared_ptr<Element>& child);
        const std::vector<std::shared_ptr<Element>>& GetChildren() const { return children_; }

        void Update(double deltaTime) override;
        float4 Arrange(float4 available) override;
        void CollectRenderCommands(std::vector<UIRenderCommand>& commands, const mat3& parentTransform) override;
        std::shared_ptr<Element> HitTest(float2 point) override;
        void SetEngine(Engine* engine) override;

    protected:
        std::vector<std::shared_ptr<Element>> children_;
    };
}
