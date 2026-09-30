//
// Created by Kyle Smith on 2026-09-30.
//

#include "pipeline/wgpu/pipeline/MeshPipeline.h"
#include "pipeline/wgpu/WGPURenderer.h"

glengine::pipeline::wgpu::MeshPipeline::MeshPipeline(WGPUDevice device, WGPURenderPipeline pipeline,
    std::vector<WGPUBindGroupLayout> layouts, WGPUBindGroup universalGroup, uint32_t dataSize) : Pipeline(device, std::move(layouts), universalGroup) {
    _pipeline = pipeline;
    _immediateDataSize = dataSize;
}

glengine::pipeline::wgpu::MeshPipeline::MeshPipeline(MeshPipeline &other) : Pipeline(other) {
    _pipeline = other._pipeline;
    _immediateDataSize = other._immediateDataSize;
    wgpuRenderPipelineAddRef(_pipeline);
}

glengine::pipeline::wgpu::MeshPipeline::~MeshPipeline() {
    wgpuRenderPipelineRelease(_pipeline);
}

void glengine::pipeline::wgpu::MeshPipeline::DispatchMeshTasks(const RenderBundle &bundle, uint32_t x, uint32_t y, uint32_t z, const void *immediateData) {
    auto pass = createPass(bundle);
    if (_immediateDataSize > 0) {
        wgpuRenderPassEncoderSetImmediates(pass, 0, immediateData, _immediateDataSize);
    }

    wgpuRenderPassEncoderDrawMeshTasks(pass, x, y, z);
}

void glengine::pipeline::wgpu::MeshPipeline::DispatchMeshTasksIndirect(const RenderBundle &bundle, WGPUBuffer indirectBuffer, uint64_t offset, const void *immediateData) {
    auto pass = createPass(bundle);
    if (_immediateDataSize > 0) {
        wgpuRenderPassEncoderSetImmediates(pass, 0, immediateData, _immediateDataSize);
    }

    wgpuRenderPassEncoderDrawMeshTasksIndirect(pass, indirectBuffer, offset);
}

std::shared_ptr<glengine::pipeline::wgpu::MeshPipeline> glengine::pipeline::wgpu::MeshPipeline::CreateInstance() {
    return std::make_shared<MeshPipeline>(*this);
}

WGPURenderPassEncoder glengine::pipeline::wgpu::MeshPipeline::createPass(const RenderBundle &bundle) {

    auto pass = bundle.passEncoder;
    wgpuRenderPassEncoderSetPipeline(pass, _pipeline);
    for (int i = 1; i < _groups.size(); i++) {
        wgpuRenderPassEncoderSetBindGroup(pass, i, _groups[i], 0, nullptr);
    }

    return pass;
}
