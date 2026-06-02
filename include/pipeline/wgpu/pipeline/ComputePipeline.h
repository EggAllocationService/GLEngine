//
// Created by Kyle Smith on 2026-06-02.
//
#pragma once
#include "pipeline/wgpu/pipeline/Pipeline.h"
#include "glengine_export.h"

namespace glengine::pipeline::wgpu {
    struct ComputeBundle {
        WGPUCommandEncoder cmd;
        WGPUComputePassEncoder encoder;
    };

    class GLENGINE_EXPORT ComputePipeline : public Pipeline {
    public:
        ComputePipeline(WGPUDevice device, WGPUComputePipeline pipeline, std::vector<WGPUBindGroupLayout> layouts, uint32_t immediateDataSize);
        explicit ComputePipeline(ComputePipeline &other);
        ~ComputePipeline();
        void DispatchWorkgroups(ComputeBundle& bundle, int x, int y, int z, void* immediateData);
        void DispatchWorkgroupsIndirect(ComputeBundle& bundle, WGPUBuffer indirectArgs, int offset, void* immediateData);

        std::shared_ptr<ComputePipeline> CreateInstance();
    private:
        WGPUComputePipeline _pipeline;
        uint32_t _immediateDataSize;
    };
}