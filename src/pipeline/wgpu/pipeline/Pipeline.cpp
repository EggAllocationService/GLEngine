//
// Created by Kyle Smith on 2026-05-29.
//

#include "pipeline/wgpu/pipeline/Pipeline.h"
namespace glengine::pipeline::wgpu {
    Pipeline::Pipeline(WGPUDevice device, std::vector<WGPUBindGroupLayout> layouts,
        WGPUBindGroup universalGroup) {
        _device = device;
        _layouts = std::move(layouts);
        _dirty = std::vector(_layouts.size(), false);
        _groups = std::vector<WGPUBindGroup>(_layouts.size(), nullptr);
        _entries.resize(_layouts.size());
        if (universalGroup != nullptr) {
            _groups[0] = universalGroup;
            wgpuBindGroupAddRef(universalGroup);
        }
    }

    Pipeline::Pipeline(Pipeline &other) {
        _groups = std::vector(other._groups);
        _entries = std::vector(other._entries);
        _layouts = std::vector(other._layouts);
        _device = other._device;
        _dirty = other._dirty;

        for (int i = 0; i < _groups.size(); i++) {
            if (_groups[i] != nullptr) {
                wgpuBindGroupAddRef(_groups[i]);
            }
            if (_layouts[i] != nullptr) {
                wgpuBindGroupLayoutAddRef(_layouts[i]);
            }
        }
    }

    Pipeline::~Pipeline() {
        for (int i = 0; i < _groups.size(); i++) {
            wgpuBindGroupRelease(_groups[i]);
            wgpuBindGroupLayoutRelease(_layouts[i]);
        }
    }

    void Pipeline::SetBinding(int group, WGPUBindGroupEntry entry) {
        auto& vector = _entries[group];
        if (vector.size() < entry.binding + 1) {
            vector.resize(entry.binding + 1);
        }
        vector[entry.binding] = entry;
        _dirty[group] = true;
    }

    void Pipeline::SetBinding(int group, unsigned int slot, WGPUBuffer buffer) {
        SetBinding(group, {
            .nextInChain = nullptr,
            .binding = slot,
            .buffer = buffer,
            .offset = 0,
            .size = WGPU_WHOLE_SIZE,
            .sampler = nullptr,
            .textureView = nullptr
        });
    }

    void Pipeline::SetBinding(int group, unsigned int slot, WGPUBuffer buffer, unsigned int offset, unsigned int size) {
        SetBinding(group, {
            .nextInChain = nullptr,
            .binding = slot,
            .buffer = buffer,
            .offset = offset,
            .size = size,
            .sampler = nullptr,
            .textureView = nullptr
        });
    }

    void Pipeline::SetBinding(int group, unsigned int slot, WGPUTextureView texture) {
        SetBinding(group, {
            .nextInChain = nullptr,
            .binding = slot,
            .buffer = nullptr,
            .offset = 0,
            .size = 0,
            .sampler = nullptr,
            .textureView = texture
        });
    }

    void Pipeline::CommitBindings() {
        for (int i = 0; i < _groups.size(); i++) {
            if (!_dirty[i]) continue;

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
}