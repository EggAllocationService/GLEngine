//
// Created by Kyle Smith on 2026-06-02.
//

#include "pipeline/wgpu/WrappedBuffer.h"

namespace glengine::pipeline::wgpu {
    WrappedBuffer::WrappedBuffer() {
        buffer = nullptr;
    }

    WrappedBuffer::WrappedBuffer(WGPUBuffer buffer) {
        this->buffer = buffer;
    }

    WrappedBuffer::~WrappedBuffer() {
        if (buffer != nullptr) {
            wgpuBufferRelease(buffer);
        }
    }

    WrappedBuffer::WrappedBuffer(const WrappedBuffer &other) {
        buffer = other.buffer;
        wgpuBufferRelease(buffer);
    }

    WrappedBuffer & WrappedBuffer::operator=(const WrappedBuffer &other) {
        if (buffer != nullptr) {
            wgpuBufferRelease(buffer);
        }
        buffer = other.buffer;
        wgpuBufferAddRef(buffer);
        return *this;
    }
}
