//
// Created by Kyle Smith on 2026-06-02.
//
#pragma once
#include "GPUMesh.h"

namespace glengine::pipeline::wgpu {
    class GPUTexture {
    public:
        GPUTexture(WGPUTexture texture, WGPUTextureFormat format, unsigned int width, unsigned int height) {
            this->texture = texture;
            this->view = wgpuTextureCreateView(texture, nullptr);
            this->format = format;
            this->width = width;
            this->height = height;
        }
        GPUTexture(const GPUTexture& other) {
            this->texture = other.texture;
            this->view = other.view;
            this->format = other.format;
            this->width = other.width;
            this->height = other.height;

            wgpuTextureAddRef(texture);
            wgpuTextureViewAddRef(view);
        }
        ~GPUTexture() {
            wgpuTextureRelease(texture);
            wgpuTextureViewRelease(view);
        }

        [[nodiscard]] WGPUTextureFormat GetFormat() const { return format; }
        [[nodiscard]] unsigned int GetWidth() const { return width; }
        [[nodiscard]] unsigned int GetHeight() const { return height; }

        operator WGPUTextureView() const {
            return view;
        }
        operator WGPUTexture() const {
            return texture;
        }
    private:
        WGPUTexture texture;
        WGPUTextureView view;
        WGPUTextureFormat format;
        unsigned int width;
        unsigned int height;
    };
}
