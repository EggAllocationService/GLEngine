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
    if (!bundle.valid) return;
    auto pass = createPass(bundle);

    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, mesh.GetVertices(), 0, mesh.GetVertexCount() * mesh.GetVertexStride());
    if (_immediateDataSize != 0 && immediateData) {
        wgpuRenderPassEncoderSetImmediates(pass, 0, _immediateDataSize, immediateData);
    }

    if (mesh.IsIndexed()) {
        wgpuRenderPassEncoderSetIndexBuffer(pass, mesh.GetIndices(), WGPUIndexFormat_Uint32, 0, mesh.GetIndexCount() * sizeof(unsigned int));
        wgpuRenderPassEncoderDrawIndexed(pass, mesh.GetIndexCount(), 1, 0, 0, 0);
    }
    else {
        wgpuRenderPassEncoderDraw(pass, mesh.GetVertexCount(), 0, 0, 0);
    }
}

void glengine::pipeline::wgpu::RenderPipeline::DrawMeshInstanced(const RenderBundle &bundle, const GPUMesh &mesh,
    int instanceCount) {
    if (!bundle.valid) return;
    auto pass = createPass(bundle);
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, mesh.GetVertices(), 0, mesh.GetVertexCount() * mesh.GetVertexStride());
    if (mesh.IsIndexed()) {
        wgpuRenderPassEncoderSetIndexBuffer(pass, mesh.GetIndices(), WGPUIndexFormat_Uint32, 0, mesh.GetIndexCount() * sizeof(unsigned int));
        wgpuRenderPassEncoderDrawIndexed(pass, mesh.GetIndexCount(), instanceCount, 0, 0, 0);
    }
    else {
        wgpuRenderPassEncoderDraw(pass, mesh.GetVertexCount(), instanceCount, 0, 0);
    }
}

void glengine::pipeline::wgpu::RenderPipeline::DrawMeshInstancedIndirect(const RenderBundle &bundle, const GPUMesh &mesh, WGPUBuffer indirectBuffer, void *immediateData) {
    if (!bundle.valid) return;

    auto pass = createPass(bundle);
    if (_immediateDataSize > 0 && immediateData != nullptr) {
        wgpuRenderPassEncoderSetImmediates(pass, 0,  _immediateDataSize, immediateData);
    }

    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, mesh.GetVertices(), 0, mesh.GetVertexCount() * mesh.GetVertexStride());
    if (mesh.IsIndexed()) {
        wgpuRenderPassEncoderSetIndexBuffer(pass, mesh.GetIndices(), WGPUIndexFormat_Uint32, 0, mesh.GetIndexCount() * sizeof(unsigned int));

        wgpuRenderPassEncoderDrawIndexedIndirect(bundle.passEncoder, indirectBuffer, 0);
    } else {
        wgpuRenderPassEncoderDrawIndirect(bundle.passEncoder, indirectBuffer, 0);
    }
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

    auto pass = bundle.passEncoder;
    wgpuRenderPassEncoderSetPipeline(pass, _pipeline);
    for (int i = 1; i < _groups.size(); i++) {
        wgpuRenderPassEncoderSetBindGroup(pass, i, _groups[i], 0, nullptr);
    }

    return pass;
}
