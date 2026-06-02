//
// Created by Kyle Smith on 2026-06-02.
//

#include "pipeline/wgpu/pipeline/ComputePipeline.h"

#include <utility>
#include <webgpu/wgpu.h>

namespace glengine::pipeline::wgpu {
    ComputePipeline::ComputePipeline(WGPUDevice device, WGPUComputePipeline pipeline,
        std::vector<WGPUBindGroupLayout> layouts, uint32_t immediateDataSize) : Pipeline(device, std::move(layouts), nullptr) {
        _immediateDataSize = immediateDataSize;
        _pipeline = pipeline;
    }

    ComputePipeline::ComputePipeline(ComputePipeline &other) : Pipeline(other) {
        _pipeline = other._pipeline;
        _immediateDataSize = other._immediateDataSize;
        wgpuComputePipelineAddRef(_pipeline);
    }

    ComputePipeline::~ComputePipeline() {
        wgpuComputePipelineRelease(_pipeline);
    }

    void ComputePipeline::DispatchWorkgroups(ComputeBundle& bundle, int x, int y, int z, void *immediateData) {
        wgpuComputePassEncoderSetPipeline(bundle.encoder, _pipeline);
        for (int i = 0; i < _groups.size(); i++) {
            wgpuComputePassEncoderSetBindGroup(bundle.encoder, i, _groups[i], 0, nullptr);
        }
        if (_immediateDataSize > 0) {
            wgpuComputePassEncoderSetImmediates(bundle.encoder, 0, _immediateDataSize, immediateData);
        }

        wgpuComputePassEncoderDispatchWorkgroups(bundle.encoder, x, y, z);
    }

    void ComputePipeline::DispatchWorkgroupsIndirect(ComputeBundle& bundle, WGPUBuffer indirectArgs, int offset, void *immediateData) {
        wgpuComputePassEncoderSetPipeline(bundle.encoder, _pipeline);
        for (int i = 0; i < _groups.size(); i++) {
            wgpuComputePassEncoderSetBindGroup(bundle.encoder, i, _groups[i], 0, nullptr);
        }
        if (_immediateDataSize > 0) {
            wgpuComputePassEncoderSetImmediates(bundle.encoder, 0, _immediateDataSize, immediateData);
        }

        wgpuComputePassEncoderDispatchWorkgroupsIndirect(bundle.encoder, indirectArgs, offset);
    }

    std::shared_ptr<ComputePipeline> ComputePipeline::CreateInstance() {
        return std::make_shared<ComputePipeline>(*this);
    }
}
