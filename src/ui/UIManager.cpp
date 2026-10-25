//
// Created by Kyle Smith on 2026-10-24.
//

#include "ui/UIManager.h"

#include <algorithm>
#include <cstring>

#include "Engine.h"
#include "GLMath.h"
#include "Shaders.h"
#include "pipeline/wgpu/WGPURenderer.h"
#include "pipeline/wgpu/pipeline/RenderPipeline.h"
#include "3d/text/Font.h"

namespace glengine::ui {
    namespace {
        struct UIImmediate {
            mat4 transform;
            float4 color;
        };

        mat4 Mat3ToMat4(const mat3& m) {
            mat4 result = mat4::identity();
            // mat3 is column-major: columns 0 and 1 are the 2D basis, column 2 is translation
            result.data[0] = m.data[0];
            result.data[1] = m.data[1];
            result.data[4] = m.data[3];
            result.data[5] = m.data[4];
            result.data[12] = m.data[6];
            result.data[13] = m.data[7];
            return result;
        }
    }

    UIManager::UIManager(Engine* engine) : engine_(engine) {
        rootView_ = std::make_shared<View>();
        rootView_->engine_ = engine_;

        // create the standard system cursors up front
        cursors_[Cursor::Pointer] = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);
        cursors_[Cursor::Text] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        cursors_[Cursor::Crosshair] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
        cursors_[Cursor::ResizeHorizontal] = glfwCreateStandardCursor(GLFW_RESIZE_EW_CURSOR);
        cursors_[Cursor::ResizeVertical] = glfwCreateStandardCursor(GLFW_RESIZE_NS_CURSOR);
        cursors_[Cursor::ResizeDiagonalRight] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
        cursors_[Cursor::ResizeDiagonalLeft] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
        cursors_[Cursor::ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
        cursors_[Cursor::NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);

        BuildPipeline();
    }

    UIManager::~UIManager() {
        if (uiTextBindGroup_ != nullptr) {
            wgpuBindGroupRelease(uiTextBindGroup_);
        }
        for (const auto& [cursor, glfwCursor] : cursors_) {
            if (glfwCursor != nullptr) {
                glfwDestroyCursor(glfwCursor);
            }
        }
    }

    void UIManager::ApplyCursor(Cursor cursor) {
        if (cursor == activeCursor_) {
            return;
        }
        activeCursor_ = cursor;

        GLFWcursor* glfwCursor = nullptr;
        if (cursor != Cursor::Default) {
            const auto found = cursors_.find(cursor);
            if (found != cursors_.end()) {
                glfwCursor = found->second;
            }
        }
        glfwSetCursor(engine_->GetWindow(), glfwCursor);
    }

    void UIManager::BuildPipeline() {
        auto renderer = engine_->GetRenderer();
        auto shader = renderer->CompileShader(embed_UI_wgsl);

        WGPUVertexAttribute attribute = {
            .nextInChain = nullptr,
            .format = WGPUVertexFormat_Float32x2,
            .offset = 0,
            .shaderLocation = 0
        };
        WGPUVertexBufferLayout layout = {
            .nextInChain = nullptr,
            .stepMode = WGPUVertexStepMode_Vertex,
            .arrayStride = sizeof(float) * 2,
            .attributeCount = 1,
            .attributes = &attribute
        };

        auto extras = pipeline::wgpu::RenderPipelineExtras {
            .polygonMode = WGPUPolygonMode_Fill,
            .depthMode = WGPUCompareFunction_Always,
            .cullMode = WGPUCullMode_None,
            .primitiveTopology = WGPUPrimitiveTopology_TriangleList,
            .depthDisabled = true
        };

        pipeline_ = renderer->BuildRenderPipeline("BuiltinUI", shader, &layout, {}, sizeof(UIImmediate), &extras);

        std::vector<float2> vertices = {
            {0, 0},
            {1, 0},
            {0, 1},
            {1, 0},
            {1, 1},
            {0, 1}
        };
        std::vector<unsigned int> indices = {0, 1, 2, 3, 4, 5};
        unitQuad_ = renderer->UploadIndexedMesh(vertices, indices);

        uiTextUniforms_ = std::make_unique<pipeline::wgpu::WrappedBuffer>(
            renderer->CreateRawBuffer("UI Text Uniforms", WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
                                      sizeof(pipeline::wgpu::RenderUniforms)));
    }

    void UIManager::Update(double deltaTime) {
        rootView_->Update(deltaTime);

        const auto windowSize = engine_->GetWindowSize();
        if (windowSize.x != lastWindowSize_.x || windowSize.y != lastWindowSize_.y) {
            lastWindowSize_ = windowSize;
        }

        rootView_->Arrange({0.0f, 0.0f, static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
    }

    void UIManager::Render(pipeline::wgpu::FrameBundle& frame) {
        std::vector<UIRenderCommand> commands;
        commands.reserve(64);
        rootView_->CollectRenderCommands(commands, mat3::identity());

        if (commands.empty()) {
            return;
        }

        std::stable_sort(commands.begin(), commands.end(), [](const UIRenderCommand& a, const UIRenderCommand& b) {
            return a.zIndex < b.zIndex;
        });

        const auto windowSize = engine_->GetWindowSize();
        const auto projection = math::orthographicMatrix(0.0f, static_cast<float>(windowSize.x), 0.0f,
                                                         static_cast<float>(windowSize.y), -1.0f, 1.0f);

        WGPURenderPassColorAttachment attachment = {
            .nextInChain = nullptr,
            .view = *frame.colorTextures[frame.presentIndex],
            .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
            .resolveTarget = nullptr,
            .loadOp = WGPULoadOp_Load,
            .storeOp = WGPUStoreOp_Store,
            .clearValue = {0, 0, 0, 0}
        };
        WGPURenderPassDescriptor descriptor = {
            .nextInChain = nullptr,
            .label = {},
            .colorAttachmentCount = 1,
            .colorAttachments = &attachment,
            .depthStencilAttachment = nullptr,
            .occlusionQuerySet = nullptr,
            .timestampWrites = nullptr
        };

        auto renderer = engine_->GetRenderer();
        auto pass = wgpuCommandEncoderBeginRenderPass(frame.encoder, &descriptor);
        wgpuRenderPassEncoderSetBindGroup(pass, 0, renderer->GetUniversalBindGroup(), 0, nullptr);

        pipeline::wgpu::RenderBundle bundle {
            .passEncoder = pass,
            .targetTexture = *frame.colorTextures[frame.presentIndex],
            .depthTexture = nullptr,
            .valid = true
        };

        bool textUniformsUploaded = false;
        for (const auto& command : commands) {
            if (command.type == UIRenderCommand::Type::Text && !textUniformsUploaded) {
                // lazily create the UI text bind group once the light tracker has
                // populated the shared lights binding in the universal layout
                if (uiTextBindGroup_ == nullptr) {
                    auto lightsEntry = renderer->GetUniversalEntry(1);
                    if (lightsEntry.buffer != nullptr) {
                        WGPUBindGroupEntry entries[2] = {WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT};
                        entries[0].binding = 0;
                        entries[0].buffer = *uiTextUniforms_;
                        entries[1] = lightsEntry;

                        WGPUBindGroupDescriptor desc = {
                            .nextInChain = nullptr,
                            .label = {},
                            .layout = renderer->GetUniversalBindGroupLayout(),
                            .entryCount = 2,
                            .entries = &entries[0]
                        };
                        uiTextBindGroup_ = wgpuDeviceCreateBindGroup(renderer->GetDevice(), &desc);
                    }
                }

                pipeline::wgpu::RenderUniforms uniforms = {
                    .projectionViewMatrix = projection,
                    .cameraMatrix = mat4::identity(),
                    .viewMatrix = mat4::identity(),
                    .lightCount = 0,
                    .time = 0.0f
                };
                renderer->GetTransferManager()->Transfer(*uiTextUniforms_, 0, &uniforms, sizeof(uniforms));

                // UI text reads its own uniforms; rectangles don't use group 0
                wgpuRenderPassEncoderSetBindGroup(pass, 0,
                    uiTextBindGroup_ != nullptr ? uiTextBindGroup_ : renderer->GetUniversalBindGroup(),
                    0, nullptr);
                textUniformsUploaded = true;
            }

            RenderCommand(command, bundle, projection);
        }

        wgpuRenderPassEncoderEnd(pass);
        wgpuRenderPassEncoderRelease(pass);
    }

    void UIManager::RenderCommand(const UIRenderCommand& command, pipeline::wgpu::RenderBundle& bundle,
                                  const mat4& projection) {
        const auto model = Mat3ToMat4(command.transform);

        if (command.type == UIRenderCommand::Type::Rectangle) {
            UIImmediate immediate = {
                .transform = projection * model,
                .color = command.color
            };
            pipeline_->DrawMesh(bundle, *unitQuad_, &immediate);
        } else if (command.type == UIRenderCommand::Type::Text) {
            if (!command.textMesh || !command.font) return;
            auto fontPipeline = command.font->GetUIPipeline();
            fontPipeline->DrawMesh(bundle, *command.textMesh, &model);
        }
    }

    std::shared_ptr<Element> UIManager::HitTest(float2 point) {
        return rootView_->HitTest(point);
    }

    void UIManager::SetFocus(const std::shared_ptr<Element>& element) {
        if (auto oldFocus = focusedElement_.lock()) {
            if (oldFocus == element) return;
            oldFocus->OnFocusChanged(false);
        }
        focusedElement_ = element;
        if (element) {
            element->OnFocusChanged(true);
        }
    }

    void UIManager::ClearFocus() {
        SetFocus(nullptr);
    }

    bool UIManager::HandleKeyPressed(int keyCode) {
        if (auto focused = focusedElement_.lock()) {
            if (focused->IsVisible() && focused->IsEnabled()) {
                focused->OnKeyPressed(keyCode);
                return true;
            }
        }
        return false;
    }

    void UIManager::HandleMouseMove(float2 point) {
        if (auto captured = capturedElement_.lock()) {
            if (captured->IsVisible() && captured->IsEnabled()) {
                captured->OnMouseDrag(captured->ToLocal(point));
            }
            return;
        }

        auto hit = HitTest(point);
        auto current = hoveredElement_.lock();
        if (hit != current) {
            if (current) {
                current->OnMouseLeave();
            }
            if (hit) {
                hit->OnMouseEnter();
            }
            hoveredElement_ = hit;
        }

        ApplyCursor(hit ? hit->cursor : Cursor::Default);
    }

    void UIManager::HandleMouseClick(int button, bool down, float2 point) {
        if (down) {
            auto hit = HitTest(point);
            // only elements that opt in to keyboard focus may take it
            if (hit && hit->focusable) {
                SetFocus(hit);
            } else {
                ClearFocus();
            }
            if (hit) {
                capturedElement_ = hit;
                hit->OnMouseDown(button, hit->ToLocal(point));
            }
        } else {
            if (auto captured = capturedElement_.lock()) {
                captured->OnMouseUp(button, captured->ToLocal(point));
            }
            capturedElement_.reset();
        }
    }
}
