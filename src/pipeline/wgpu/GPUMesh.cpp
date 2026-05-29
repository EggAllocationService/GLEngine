//
// Created by Kyle Smith on 2026-05-28.
//

#include "pipeline/wgpu/GPUMesh.h"

glengine::pipeline::wgpu::GPUMesh::GPUMesh(WGPUBuffer buffer, int vertexCount, int id) {
    this->buffer = buffer;
    this->vertexCount = vertexCount;
    this->id = id;
}

glengine::pipeline::wgpu::GPUMesh::~GPUMesh() {
    wgpuBufferRelease(buffer);
}

WGPUBuffer glengine::pipeline::wgpu::GPUMesh::GetBuffer() const {
    return buffer;
}

int glengine::pipeline::wgpu::GPUMesh::GetVertexCount() const {
    return vertexCount;
}
