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
        _groups[0] = universalGroup;
    }

    void Pipeline::SetBinding(int group, WGPUBindGroupEntry entry) {
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

    void Pipeline::CommitBindings() {
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
}