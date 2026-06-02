//
// Created by Kyle Smith on 2026-06-02.
//
#pragma once
#include "TypedGPUBuffer.h"

namespace glengine::pipeline::wgpu {
    struct WrappedBuffer {
        WrappedBuffer();
        WrappedBuffer(WGPUBuffer buffer);
        ~WrappedBuffer();
        WrappedBuffer(const WrappedBuffer&);
        WrappedBuffer& operator=(const WrappedBuffer&);

        operator WGPUBuffer() {
            return buffer;
        }
    private:
        WGPUBuffer buffer;
    };
}


