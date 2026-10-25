//
// Created by Kyle Smith on 2026-10-24.
//
#pragma once

#include <memory>
#include <vector>

#include "ui/UIElement.h"
#include "Vectors.h"
#include "glengine_export.h"
#include "webgpu/wgpu.h"

namespace glengine {
    class Engine;
}

namespace glengine::pipeline::wgpu {
    struct FrameBundle;
    struct RenderBundle;
    struct WrappedBuffer;
    class RenderPipeline;
    class GPUMesh;
}

namespace glengine::ui {
    class View;

    /// Manages the UI tree: layout, input, and rendering.
    class GLENGINE_EXPORT UIManager {
    public:
        explicit UIManager(Engine* engine);
        ~UIManager();

        /// The root view fills the entire window.
        std::shared_ptr<View> GetRootView() const { return rootView_; }

        /// Call once per frame.
        void Update(double deltaTime);

        /// Call after the 3D scene and post-processing have been rendered.
        void Render(pipeline::wgpu::FrameBundle& frame);

        /// Hit-test the UI at a pixel-space position (bottom-left origin).
        std::shared_ptr<Element> HitTest(float2 point);

        /// Focus handling
        void SetFocus(const std::shared_ptr<Element>& element);
        std::shared_ptr<Element> GetFocusedElement() const { return focusedElement_.lock(); }
        void ClearFocus();

        /// Forward keyboard input to the focused element.
        bool HandleKeyPressed(int keyCode);

        /// Notify the UI of mouse motion in FREE mode.
        void HandleMouseMove(float2 point);

        /// Notify the UI of a mouse button event.
        void HandleMouseClick(int button, bool down, float2 point);

    private:
        void BuildPipeline();
        void RenderCommand(const UIRenderCommand& command, pipeline::wgpu::RenderBundle& bundle,
                           const mat4& projection);

        Engine* engine_;
        std::shared_ptr<View> rootView_;
        std::weak_ptr<Element> focusedElement_;
        std::weak_ptr<Element> hoveredElement_;
        std::weak_ptr<Element> capturedElement_;

        std::shared_ptr<pipeline::wgpu::RenderPipeline> pipeline_;
        std::shared_ptr<pipeline::wgpu::GPUMesh> unitQuad_;

        /// Separate uniforms/bind group for UI text so the 3D camera uniforms
        /// are never touched by the UI pass.
        std::unique_ptr<pipeline::wgpu::WrappedBuffer> uiTextUniforms_;
        WGPUBindGroup uiTextBindGroup_ = nullptr;

        int2 lastWindowSize_ = int2(0, 0);
    };
}