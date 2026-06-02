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