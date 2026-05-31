//
// Created by Kyle Smith on 2026-05-28.
//

#include "../../../../include/pipeline/wgpu/pipeline/RenderPipeline.h"
#include "pipeline/wgpu/WGPURenderer.h"


glengine::pipeline::wgpu::RenderPipeline::RenderPipeline(WGPUDevice device, WGPURenderPipeline pipeline,
    std::vector<WGPUBindGroupLayout> layouts, WGPUBindGroup universalGroup, uint32_t dataSize) : Pipeline(device, std::move(layouts), universalGroup) {
    _pipeline = pipeline;
    _immediateDataSize = dataSize;
}

void glengine::pipeline::wgpu::RenderPipeline::DrawMesh(const RenderBundle &bundle, const GPUMesh &mesh,
                                                        const void *immediateData) {
    auto pass = createPass(bundle);

    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, mesh.GetVertices(), 0, mesh.GetVertexCount() * sizeof(Vertex));
    if (_immediateDataSize != 0 && immediateData) {
        wgpuRenderPassEncoderSetImmediates(pass, 0, _immediateDataSize, immediateData);
    }

    if (mesh.IsIndexed()) {
        wgpuRenderPassEncoderSetIndexBuffer(pass, mesh.GetIndices(), WGPUIndexFormat_Uint32, 0, mesh.GetIndexCount());
        wgpuRenderPassEncoderDrawIndexed(pass, mesh.GetIndexCount(), 1, 0, 0, 0);
    }
    else {
        wgpuRenderPassEncoderDraw(pass, mesh.GetVertexCount(), 0, 0, 0);
    }
    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);
}

void glengine::pipeline::wgpu::RenderPipeline::DrawMeshInstanced(const RenderBundle &bundle, const GPUMesh &mesh,
    int instanceCount) {
    auto pass = createPass(bundle);
    auto indices = mesh.GetIndices();
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, mesh.GetVertices(), 0, mesh.GetVertexCount() * sizeof(Vertex));
    if (mesh.IsIndexed()) {
        wgpuRenderPassEncoderSetIndexBuffer(pass, mesh.GetIndices(), WGPUIndexFormat_Uint32, 0, mesh.GetIndexCount());
        wgpuRenderPassEncoderDrawIndexed(pass, mesh.GetIndexCount(), instanceCount, 0, 0, 0);
    }
    else {
        wgpuRenderPassEncoderDraw(pass, mesh.GetVertexCount(), instanceCount, 0, 0);
    }
    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);
}

std::shared_ptr<glengine::pipeline::wgpu::RenderPipeline> glengine::pipeline::wgpu::RenderPipeline::CreateInstance() {
    return std::make_shared<RenderPipeline>(*this);
}

glengine::pipeline::wgpu::RenderPipeline::RenderPipeline(RenderPipeline &other) : Pipeline(other) {
    _pipeline = other._pipeline;
    _immediateDataSize = other._immediateDataSize;
    wgpuRenderPipelineAddRef(_pipeline);
}

glengine::pipeline::wgpu::RenderPipeline::~RenderPipeline() {
    wgpuRenderPipelineRelease(_pipeline);
}

WGPURenderPassEncoder glengine::pipeline::wgpu::RenderPipeline::createPass(const RenderBundle &bundle) {
    auto colorAttachment = WGPURenderPassColorAttachment {
        .nextInChain = nullptr,
        .view = bundle.targetTexture,
        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
        .resolveTarget = nullptr,
        .loadOp = WGPULoadOp_Load,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = WGPUColor(1, 1, 1, 1)
    };
    auto depthAttachment = WGPURenderPassDepthStencilAttachment {
        .nextInChain = nullptr,
        .view = bundle.depthTexture,
        .depthLoadOp = WGPULoadOp_Load,
        .depthStoreOp = WGPUStoreOp_Store,
        .depthClearValue = 0,
        .depthReadOnly = false,
        .stencilLoadOp = WGPULoadOp_Undefined,
        .stencilStoreOp = WGPUStoreOp_Undefined,
        .stencilClearValue = 0,
        .stencilReadOnly = true
    };

    WGPURenderPassDescriptor desc = {
        .nextInChain = nullptr,
        .label = {},
        .colorAttachmentCount = 1,
        .colorAttachments = &colorAttachment,
        .depthStencilAttachment = &depthAttachment,
        .occlusionQuerySet = nullptr,
        .timestampWrites = nullptr
    };
    auto pass = wgpuCommandEncoderBeginRenderPass(bundle.encoder, &desc);

    wgpuRenderPassEncoderSetPipeline(pass, _pipeline);
    for (int i = 0; i < _groups.size(); i++) {
        wgpuRenderPassEncoderSetBindGroup(pass, i, _groups[i], 0, nullptr);
    }

    return pass;
}
