//
// Created by Kyle Smith on 2026-05-28.
//

#include "pipeline/wgpu/RenderPipeline.h"
#include "pipeline/wgpu/WGPURenderer.h"


glengine::pipeline::wgpu::RenderPipeline::RenderPipeline(WGPUDevice device, WGPURenderPipeline pipeline,
    std::vector<WGPUBindGroupLayout> layouts, WGPUBindGroup universalGroup, uint32_t dataSize) {
    _device = device;
    _pipeline = pipeline;
    _immediateDataSize = dataSize;
    _layouts = std::move(layouts);
    _dirty = std::vector(_layouts.size(), false);
    _groups = std::vector<WGPUBindGroup>(_layouts.size(), nullptr);
    _entries.resize(_layouts.size());
    _groups[0] = universalGroup;
}

void glengine::pipeline::wgpu::RenderPipeline::DrawMesh(const RenderBundle &bundle, const GPUMesh &mesh,
    const void *immediateData) {
    auto pass = createPass(bundle);

    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, mesh.GetBuffer(), 0, mesh.GetVertexCount() * sizeof(Vertex));
    if (_immediateDataSize != 0 && immediateData) {
        wgpuRenderPassEncoderSetImmediates(pass, 0, _immediateDataSize, immediateData);
    }

    wgpuRenderPassEncoderDraw(pass, mesh.GetVertexCount(), 1, 0, 0);
    wgpuRenderPassEncoderEnd(pass);
}

void glengine::pipeline::wgpu::RenderPipeline::DrawMeshInstanced(const RenderBundle &bundle, const GPUMesh &mesh,
    int instanceCount) {
    auto pass = createPass(bundle);
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, mesh.GetBuffer(), 0, mesh.GetVertexCount() * sizeof(Vertex));
    wgpuRenderPassEncoderDraw(pass, mesh.GetVertexCount(), instanceCount, 0, 0);
    wgpuRenderPassEncoderEnd(pass);
}


void glengine::pipeline::wgpu::RenderPipeline::SetBinding(int group, WGPUBindGroupEntry entry) {
    if (group == 0) {
        return;
    }

    auto& vector = _entries[group];
    if (vector.size() < entry.binding + 1) {
        vector.resize(entry.binding + 1);
    }
    vector[entry.binding] = entry;
    _dirty[group] = true;
}

void glengine::pipeline::wgpu::RenderPipeline::CommitBindings() {
    for (int i = 1; i < _groups.size(); i++) {
        if (!_dirty[i]) return;

        if (_groups[i] != nullptr) {
            wgpuBindGroupRelease(_groups[i]);
        }

        WGPUBindGroupDescriptor desc = {
            .nextInChain = nullptr,
            .label = {},
            .layout = _layouts[i],
            .entryCount = _entries[i].size(),
            .entries = _entries[i].data(),
        };
        _groups[i] = wgpuDeviceCreateBindGroup(_device, &desc);

        _dirty[i] = false;
    }
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
