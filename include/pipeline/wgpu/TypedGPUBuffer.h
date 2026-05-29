//
// Created by Kyle Smith on 2026-05-29.
//

#pragma once
#include <utility>
#include <vector>

#include "webgpu/webgpu.h"

namespace glengine::pipeline::wgpu {
    template<typename T>
    class TypedGPUBuffer {
    public:
        TypedGPUBuffer(std::string name, WGPUDevice device, WGPUBufferUsage usage, int initialCapacity) {
            static_assert(std::is_standard_layout_v<T>, "T must be standard layout");
            static_assert(std::is_trivial_v<T>, "T must be a trivial type");
            this->device = device;
            this->name = std::move(name);
            this->queue = wgpuDeviceGetQueue(device);
            this->buffer = nullptr;
            this->bufferCapacity = 0;
            this->usage = usage | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
            this->grow(initialCapacity);
            this->storage.resize(initialCapacity);
            this->dirty = false;
        }

        ~TypedGPUBuffer() {
            wgpuBufferRelease(buffer);
        }

        T& operator[](size_t index) {
            return storage[index];
        }

        T* GetData() {
            dirty = true; // assume that someone accessing the data pointer will be writing to it
            return storage.data();
        }

        void Push(T value) {
            storage.push_back(value);
            dirty = true;
        }

        void Clear() {
            storage.clear();
            dirty = true;
        }

        void Commit(const std::function<void(WGPUBuffer buffer)>& handleResize) {
            if (!dirty) return;

            if (bufferCapacity < storage.capacity()) {
                grow(storage.capacity());
                handleResize(buffer);
            }

            wgpuQueueWriteBuffer(queue, buffer, 0, storage.data(), storage.size() * sizeof(T));

            dirty = false;
        }

        void MarkDirty() {
            dirty = true;
        }

        size_t GetCapacity() {
            return storage.capacity();
        }

        size_t GetSize() {
            return storage.size();
        }

        operator WGPUBuffer() const {
            return buffer;
        }
    private:
        void grow(size_t newCapacity) {
            WGPUBufferDescriptor desc = {
                .nextInChain = nullptr,
                .label = {
                    .data = name.data(),
                    .length = name.length(),
                },
                .usage = usage,
                .size = newCapacity * sizeof(T),
                .mappedAtCreation = false
            };
            auto newBuffer = wgpuDeviceCreateBuffer(device, &desc);

            if (this->buffer != nullptr && this->bufferCapacity > 0) {
                // copy old buffer to new buffer
                auto encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
                wgpuCommandEncoderCopyBufferToBuffer(encoder, this->buffer, 0, newBuffer, 0, this->bufferCapacity * sizeof(T));
                auto commandBundle = wgpuCommandEncoderFinish(encoder, nullptr);
                wgpuQueueSubmit(queue, 1, &commandBundle);
                wgpuBufferRelease(buffer);
            }

            this->buffer = newBuffer;
            this->bufferCapacity = newCapacity;
        }
        std::vector<T> storage;
        std::string name;
        WGPUBuffer buffer;
        WGPUDevice device;
        WGPUQueue queue;
        WGPUBufferUsage usage;
        size_t bufferCapacity; // buffer capacity in instances of T
        bool dirty;
    };
}
