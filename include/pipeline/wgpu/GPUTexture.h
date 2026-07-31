//
// Created by Kyle Smith on 2026-06-02.
//
#pragma once
#include "GPUMesh.h"

namespace glengine::pipeline::wgpu {

    enum GPUTextureType {
        Texture_1D,
        Texture_2D,
        Texture_3D,
    };

    class GPUTexture {
    public:
        GPUTexture(WGPUTexture texture, WGPUTextureFormat format, unsigned int width, unsigned int height, unsigned int depth) {
            this->texture = texture;
            this->view = wgpuTextureCreateView(texture, nullptr);
            this->format = format;
            this->width = width;
            this->height = height;
            this->depth = depth;
        }
        GPUTexture(const GPUTexture& other) {
            this->texture = other.texture;
            this->view = other.view;
            this->format = other.format;
            this->width = other.width;
            this->height = other.height;
            this->depth = other.depth;

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
        [[nodiscard]] unsigned int GetDepth() const { return depth; }
        [[nodiscard]] vec3<unsigned int> GetSize() const { return {width, height, depth}; }
        [[nodiscard]] GPUTextureType GetType() const {
            if (height == 1 && depth == 1) {
                return Texture_1D;
            } else if (depth == 1) {
                return Texture_2D;
            } else {
                return Texture_3D;
            }
        }

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
        unsigned int depth;
    };
}
