//
// Created by Kyle Smith on 2026-06-02.
//

#pragma once

#include "webgpu/webgpu.h"

#include "TransferManager.h"

namespace glengine::pipeline::wgpu {

    template<typename T>
    struct GLENGINE_EXPORT GPUPointerInternals {
        GPUPointerInternals(WGPUQueue queue, WGPUBuffer buffer) {
            static_assert(std::is_default_constructible_v<T>, "T must be default constructible");
            localCopy = T();
            this->queue = queue;
            this->buffer = buffer;
            dirty = true;
        }

        GPUPointerInternals(GPUPointerInternals& other) = delete;

        ~GPUPointerInternals() {
            wgpuBufferRelease(buffer);
        }

        T localCopy;
        WGPUBuffer buffer;
        WGPUQueue queue;
        bool dirty;
    };

    template <typename T>
    class GLENGINE_EXPORT GPUPointer {
    public:
        GPUPointer() {
            internals = nullptr;
        }

        GPUPointer(WGPUQueue queue, WGPUBuffer buf) {
            internals = std::make_shared<GPUPointerInternals<T>>(queue, buf);
        }

        T& operator*() {
            internals->dirty = true;
            return internals->localCopy;
        }

        const T& operator*() const {
            return internals->localCopy;
        }

        T* operator->() {
            if (internals == nullptr) return nullptr;
            internals->dirty = true;
            return &internals->localCopy;
        }

        const T* operator->() const {
            if (internals == nullptr) return nullptr;
            return &internals->localCopy;
        }

        operator WGPUBuffer() {
            return internals->buffer;
        }

        void Commit() {
            if (!internals->dirty) return;
            printf("Committing material\n");
            wgpuQueueWriteBuffer(internals->queue, internals->buffer, 0, &internals->localCopy, sizeof(T));
            internals->dirty = false;
        }

        void Commit(TransferSession& session) {
            if (!internals->dirty) return;
            session.Transfer(internals->buffer, 0, &internals->localCopy, sizeof(T));
            internals->dirty = false;
        }

    private:
        std::shared_ptr<GPUPointerInternals<T>> internals;
    };
}