//
// Created by Kyle Smith on 2026-08-11.
//

#include "pipeline/wgpu/WGPURenderer.h"
#include "pipeline/wgpu/post/PostPass.h"
#include "Shaders.h"


void glengine::pipeline::wgpu::post::PostPass::Execute(PostProcessEffect &effect, const void *immediateData) {
    WGPURenderPassColorAttachment attachment = {
        .nextInChain = nullptr,
        .view = *this->colorTextures[(this->source + 1) % 2],
        .depthSlice = 0,
        .resolveTarget = nullptr,
        .loadOp = WGPULoadOp_Undefined,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = {0, 0, 0, 0}
    };
    auto desc = WGPURenderPassDescriptor {
        .nextInChain = nullptr,
        .label = {},
        .colorAttachmentCount = 1,
        .colorAttachments = &attachment,
        .depthStencilAttachment = nullptr,
        .occlusionQuerySet = nullptr,
        .timestampWrites = nullptr
    };
    auto pass = wgpuCommandEncoderBeginRenderPass(this->encoder, &desc);
    wgpuRenderPassEncoderSetPipeline(pass, effect);
    wgpuRenderPassEncoderSetBindGroup(pass, 0, universalBindGroup, 0, nullptr);
    wgpuRenderPassEncoderSetBindGroup(pass, 1, this->sceneBindGroups[this->source], 0, nullptr);
    if (effect.GetImmediateDataSize() > 0) {
        wgpuRenderPassEncoderSetImmediates(pass, 0, effect.GetImmediateDataSize(), immediateData);
    }
    wgpuRenderPassEncoderDraw(pass, 4, 1, 0, 0);
    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);

    this->source = (this->source + 1) % 2;
}

