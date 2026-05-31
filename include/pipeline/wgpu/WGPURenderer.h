//
// Created by Kyle Smith on 2026-05-28.
//

#pragma once
#include "Matrix.h"
#include "pipeline/RenderPipeline.h"
#include "webgpu/wgpu.h"
#include <span>

#include "TypedGPUBuffer.h"
#include "GLFW/glfw3.h"

#include "TransferManager.h"


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
        WGPUTextureView targetTexture;
        WGPUTextureView depthTexture;
        WGPUTexture surfaceTexture;
        bool valid;
    };

    class WGPURenderer {
    public:
        WGPURenderer(GLFWwindow* window);
        WGPUShaderModule CompileShader(const char* shaders);
        std::shared_ptr<GPUMesh> UploadMesh(const std::vector<Vertex>& vertices);
        std::shared_ptr<GPUMesh> UploadIndexedMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

        std::shared_ptr<RenderPipeline> GetRenderPipelineByName(const std::string& name);
        std::shared_ptr<RenderPipeline> BuildRenderPipeline(
            std::string name,
            WGPUShaderModule shaders,
            WGPUVertexBufferLayout *vertexLayout,
            std::span<WGPUBindGroupLayoutDescriptor> bindGroups,
            int immediateDataBytes
        );

        RenderBundle BeginRendering(RenderUniforms& uniforms);
        void FinishRendering(RenderBundle bundle);

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
    };
}
