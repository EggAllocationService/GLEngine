//
// Created by Kyle Smith on 2025-11-19.
//
#include "3d/texture/StaticTexture2D.h"
#include <util/stb_image.h>
#include "engine_GLUT.h"
#include <memory>

using namespace glengine::world::texture;

StaticTexture2D::StaticTexture2D(std::istream &stream, pipeline::wgpu::WGPURenderer* renderer) {
    // figure out how big the image file is
    stream.seekg(0, std::ios::end);
    auto size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    // allocate a buffer and read file contents
    auto buffer = std::make_unique<char[]>(size);
    stream.read(buffer.get(), size);

    int x, y, n;
    auto decoded = stbi_load_from_memory((unsigned char*) buffer.get(), size, &x, &y, &n, 4);
    texture = renderer->CreateTexture("", WGPUTextureUsage_TextureBinding | WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopyDst, WGPUTextureFormat_RGBA8Unorm, x, y);

    auto textureCopyInfo = WGPUTexelCopyTextureInfo {
        .texture = *texture,
        .mipLevel = 0,
        .origin = {0, 0, 0},
        .aspect = WGPUTextureAspect_All
    };

    auto bufferInfo = WGPUTexelCopyBufferLayout {
        .offset = 0,
        .bytesPerRow = static_cast<unsigned int>(x * 4),
        .rowsPerImage = static_cast<unsigned int>(y)
    };
    auto extent = WGPUExtent3D {
        .width = static_cast<unsigned int>(x),
        .height = static_cast<unsigned int>(y),
        .depthOrArrayLayers = 1
    };;

    wgpuQueueWriteTexture(wgpuDeviceGetQueue(renderer->GetDevice()), &textureCopyInfo, decoded, x * y * 4, &bufferInfo, &extent);
}