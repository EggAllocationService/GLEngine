//
// Created by Kyle Smith on 2026-05-29.
//
#pragma once
#include <vector>

#include "webgpu/webgpu.h"
namespace glengine::pipeline::wgpu {
    class Pipeline {
    public:
        Pipeline(WGPUDevice device, std::vector<WGPUBindGroupLayout> layouts, WGPUBindGroup universalGroup);
        Pipeline(Pipeline& other);
        ~Pipeline();
        void SetBinding(int group, WGPUBindGroupEntry entry);
        void CommitBindings();
    protected:
        std::vector<WGPUBindGroup> _groups;
        std::vector<std::vector<WGPUBindGroupEntry>> _entries;
        std::vector<bool> _dirty;
        std::vector<WGPUBindGroupLayout> _layouts;
        WGPUDevice _device;
    };
}