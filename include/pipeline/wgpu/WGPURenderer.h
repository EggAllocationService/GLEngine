//
// Created by Kyle Smith on 2026-05-28.
//

#pragma once
#include "Matrix.h"
#include "pipeline/RenderPipeline.h"
#include "webgpu/wgpu.h"
#include <span>
#include "glengine_export.h"
#include "GPUPointer.h"
#include "GPUTexture.h"
#include "TypedGPUBuffer.h"
#include "GLFW/glfw3.h"
#include <atomic>

#include "TransferManager.h"
#include "WrappedBuffer.h"
#include "pipeline/ComputePipeline.h"
#include "post/PostManager.h"
#include "post/PostPass.h"


namespace glengine {
    class Engine;
}

namespace glengine::pipeline::wgpu {
    struct alignas(16) RenderUniforms {
        mat4 projectionViewMatrix;
        mat4 projectionMatrix;
        mat4 viewMatrix;
        int lightCount;
        float time;
    };

    struct Vertex {
        float3 position;
        float3 normal;
        float2 uv;
    };

    struct RenderBundle {
        WGPURenderPassEncoder passEncoder;
        WGPUTextureView targetTexture;
        WGPUTextureView depthTexture;
        bool valid;
    };

    struct FrameBundle {
        WGPUCommandEncoder encoder;
        std::shared_ptr<GPUTexture> colorTextures[2];
        std::shared_ptr<GPUTexture> depthTexture;
        int presentIndex; // which color texture to present from
    };

    struct RenderPipelineExtras {
        WGPUPolygonMode polygonMode;
        WGPUCompareFunction depthMode;
        WGPUCullMode cullMode;
        WGPUPrimitiveTopology primitiveTopology = WGPUPrimitiveTopology_TriangleList;
    };

    class GLENGINE_EXPORT WGPURenderer {
    public:
        WGPURenderer(GLFWwindow* window, Engine* engine);
        WGPUShaderModule CompileShader(const char* shaders);
        template <typename T>
        std::shared_ptr<GPUMesh> UploadMesh(const std::vector<T>& vertices) {
            auto bufferDesc = WGPUBufferDescriptor {
                .nextInChain = nullptr,
                .label = {},
                .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
                .size = sizeof(T) * vertices.size(),
                .mappedAtCreation = false
            };
            auto buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
            wgpuQueueWriteBuffer(queue, buffer, 0, vertices.data(), vertices.size() * sizeof(T));

            return std::make_shared<GPUMesh>(buffer, nullptr, vertices.size(), sizeof(T), 0, meshIdTracker++);
        }

        template<typename T>
        std::shared_ptr<GPUMesh> UploadIndexedMesh(const std::vector<T>& vertices, const std::vector<unsigned int>& indices) {
            auto verticesDesc = WGPUBufferDescriptor{
                .nextInChain = nullptr,
                .label = {},
                .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
                .size = sizeof(T) * vertices.size(),
                .mappedAtCreation = false
            };
            auto vertciesBuf = wgpuDeviceCreateBuffer(device, &verticesDesc);
            wgpuQueueWriteBuffer(queue, vertciesBuf, 0, vertices.data(), vertices.size() * sizeof(T));

            auto indicesDesc = WGPUBufferDescriptor{
                .nextInChain = nullptr,
                .label = {},
                .usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst,
                .size = sizeof(unsigned int) * indices.size(),
                .mappedAtCreation = false
            };
            auto indicesBuf = wgpuDeviceCreateBuffer(device, &indicesDesc);
            wgpuQueueWriteBuffer(queue, indicesBuf, 0, indices.data(), indices.size() * sizeof(unsigned int));

            return std::make_shared<GPUMesh>(vertciesBuf, indicesBuf, vertices.size(), sizeof(T), indices.size(), meshIdTracker++);
        }

        std::shared_ptr<RenderPipeline> GetRenderPipelineByName(const std::string& name);
        std::shared_ptr<RenderPipeline> BuildRenderPipeline(
            std::string name,
            WGPUShaderModule shaders,
            WGPUVertexBufferLayout *vertexLayout,
            std::span<WGPUBindGroupLayoutDescriptor> bindGroups,
            int immediateDataBytes,
            RenderPipelineExtras *extras
        );

        std::shared_ptr<ComputePipeline> GetComputePipelineByName(const std::string& name);
        std::shared_ptr<ComputePipeline> BuildComputePipeline(
            std::string name,
            WGPUShaderModule kernel,
            std::string_view entryPoint,
            std::span<WGPUBindGroupLayoutDescriptor> bindGroups,
            int immediateDataBytes
        );

        void SetUniversalBindGroupEntry(WGPUBindGroupEntry entry);

        FrameBundle BeginFrame();
        void PresentFrame(FrameBundle& bundle);

        post::PostPass BeginPostProcessPass(FrameBundle& bundle);
        void EndPostProcessing(post::PostPass& pass);

        std::shared_ptr<post::PostProcessEffect> CompilePostEffect(char* shader, unsigned int immediateSize);

        RenderBundle BeginRendering(FrameBundle& frame, RenderUniforms& uniforms);
        void FinishRendering(RenderBundle bundle);

        ComputeBundle BeginComputePass();
        void CommitComputePass(ComputeBundle& bundle);

        void Resize(int2 size);

        std::shared_ptr<GPUTexture> CreateTexture(std::string_view name, WGPUTextureUsage usage, WGPUTextureFormat format,
            unsigned int width, unsigned int height);
        std::shared_ptr<GPUTexture> CreateTexture(std::string_view name, WGPUTextureUsage usage, WGPUTextureFormat format,
    unsigned int width, unsigned int height, unsigned int depth);

        WrappedBuffer CreateRawBuffer(std::string_view name, WGPUBufferUsage usage, unsigned int size) const;

        template<typename T>
        std::unique_ptr<TypedGPUBuffer<T>> CreateBuffer(std::string name, WGPUBufferUsage usage, int initialCapacity) {
            return std::make_unique<TypedGPUBuffer<T>>(std::move(name), device, usage, initialCapacity);
        }

        template<typename T>
        GPUPointer<T> AllocateObject(WGPUBufferUsage usage) {
            auto desc = WGPUBufferDescriptor {
                .nextInChain = nullptr,
                .label = {},
                .usage = usage | WGPUBufferUsage_CopyDst,
                .size = sizeof(T),
                .mappedAtCreation =  false
            };

            return GPUPointer<T>(queue, wgpuDeviceCreateBuffer(device, &desc));
        }

        [[nodiscard]] WGPUDevice GetDevice() const {
            return device;
        }

        [[nodiscard]] TransferManager* GetTransferManager() const {
            return transferManager;
        }

        void BuildBuiltinPipelines();
    private:
        void rebuildUniversalBindGroup();

        WGPUSurfaceConfiguration surfConfig;
        std::vector<WGPUBindGroupEntry> universalEntries;
        std::unordered_map<std::string, std::shared_ptr<RenderPipeline>> pipelines;
        std::unordered_map<std::string, std::shared_ptr<ComputePipeline>> computePipelines;
        WGPUDevice device;
        WGPUQueue queue;
        WGPUSurface surface;

        // need two for ping-pong post-processing
        // all rendering targets texture 0
        std::shared_ptr<GPUTexture> colorTextures[2];
        std::shared_ptr<GPUTexture> depthTexture;

        WGPUBindGroupLayout universalBindGroupLayout;
        WGPUBindGroup universalBindGroup;
        WGPUBuffer renderUniformsBuffer;
        WGPUSubmissionIndex lastFrame = 0;
        TransferManager* transferManager;
        Engine* engine;
        post::PostManager *postManager;
        std::atomic<int> meshIdTracker = 0;
        unsigned long frameCounter = 0;

        bool universalDirty;

    };
}
