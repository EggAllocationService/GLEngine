//
// Created by Kyle Smith on 2026-05-28.
//

#pragma once
#include "Matrix.h"
#include "pipeline/RenderPipeline.h"
#include "webgpu/wgpu.h"
#include <span>
#include "glengine_export.h"
#include "TypedGPUBuffer.h"
#include "GLFW/glfw3.h"

#include "TransferManager.h"
#include "pipeline/ComputePipeline.h"


namespace glengine::pipeline::wgpu {
    struct alignas(16) RenderUniforms {
        mat4 projectionViewMatrix;
        mat4 projectionMatrix;
        mat4 viewMatrix;
        int lightCount;
    };

    struct Vertex {
        float3 position;
        float3 normal;
        float2 uv;
    };

    struct RenderBundle {
        WGPUCommandEncoder encoder;
        WGPURenderPassEncoder passEncoder;
        WGPUTextureView targetTexture;
        WGPUTextureView depthTexture;
        WGPUTexture surfaceTexture;
        bool valid;
    };

    class GLENGINE_EXPORT WGPURenderer {
    public:
        WGPURenderer(GLFWwindow* window);
        WGPUShaderModule CompileShader(const char* shaders);
        std::shared_ptr<GPUMesh> UploadMesh(const std::vector<Vertex>& vertices);

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
            int immediateDataBytes
        );

        std::shared_ptr<ComputePipeline> GetComputePipelineByName(const std::string& name);
        std::shared_ptr<ComputePipeline> BuildComputePipeline(
            std::string name,
            WGPUShaderModule kernel,
            std::string_view entryPoint,
            std::span<WGPUBindGroupLayoutDescriptor> bindGroups,
            int immediateDataBytes
        );

        RenderBundle BeginRendering(RenderUniforms& uniforms);
        void FinishRendering(RenderBundle bundle);

        ComputeBundle BeginComputePass();
        void CommitComputePass(ComputeBundle& bundle);

        void Resize(int2 size);

        template<typename T>
        std::unique_ptr<TypedGPUBuffer<T>> CreateBuffer(std::string name, WGPUBufferUsage usage, int initialCapacity) {
            return std::make_unique<TypedGPUBuffer<T>>(std::move(name), device, usage, initialCapacity);
        }

        [[nodiscard]] WGPUDevice GetDevice() const {
            return device;
        }

        [[nodiscard]] TransferManager* GetTransferManager() const {
            return transferManager;
        }

    private:
        void buildBuiltinPipelines();

        WGPUSurfaceConfiguration surfConfig;
        std::unordered_map<std::string, std::shared_ptr<RenderPipeline>> pipelines;
        std::unordered_map<std::string, std::shared_ptr<ComputePipeline>> computePipelines;
        WGPUDevice device;
        WGPUQueue queue;
        WGPUSurface surface;

        WGPUTexture depthTexture;
        WGPUTextureView depthTextureView;

        WGPUBindGroupLayout universalBindGroupLayout;
        WGPUBindGroup universalBindGroup;
        WGPUBuffer renderUniformsBuffer;
        WGPUSubmissionIndex lastFrame = 0;
        TransferManager* transferManager;
        std::atomic<int> meshIdTracker = 0;

    };
}
