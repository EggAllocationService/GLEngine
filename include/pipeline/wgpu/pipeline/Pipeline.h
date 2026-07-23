//
// Created by Kyle Smith on 2026-05-29.
//
#pragma once
#include <vector>
#include "glengine_export.h"

#include "webgpu/webgpu.h"
namespace glengine::pipeline::wgpu {
    class GLENGINE_EXPORT Pipeline {
    public:
        Pipeline(WGPUDevice device, std::vector<WGPUBindGroupLayout> layouts, WGPUBindGroup universalGroup);
        Pipeline(Pipeline& other);
        ~Pipeline();
        void SetBinding(int group, WGPUBindGroupEntry entry);

        void SetBinding(int group, unsigned int slot, WGPUBuffer buffer);
        void SetBinding(int group, unsigned int slot, WGPUBuffer buffer, unsigned int offset, unsigned int size);
        void SetBinding(int group, unsigned int slot, WGPUTextureView texture);
        void CommitBindings();
    protected:
        std::vector<WGPUBindGroup> _groups;
        std::vector<std::vector<WGPUBindGroupEntry>> _entries;
        std::vector<bool> _dirty;
        std::vector<WGPUBindGroupLayout> _layouts;
        WGPUDevice _device;
    };
}