//
// Created by Kyle Smith on 2026-05-28.
//

#include "pipeline/wgpu/GPUMesh.h"

#include <algorithm>

glengine::pipeline::wgpu::GPUMesh::GPUMesh(WGPUBuffer vertices, WGPUBuffer indices, int vertexCount, int vertexStride, int indexCount, int id) {
    this->vertices = vertices;
    this->indices = indices;
    this->vertexCount = vertexCount;
    this->vertexStride = vertexStride;
    this->indexCount = indexCount;
    this->vertexCapacity = vertexCount;
    this->indexCapacity = indexCount;
    this->id = id;
}

void glengine::pipeline::wgpu::GPUMesh::SetBufferCapacities(int vertices, int indices) {
    vertexCapacity = std::max(vertexCapacity, vertices);
    indexCapacity = std::max(indexCapacity, indices);
}

void glengine::pipeline::wgpu::GPUMesh::SetDrawCounts(int vertices, int indices) {
    vertexCount = vertices;
    indexCount = indices;
}

glengine::pipeline::wgpu::GPUMesh::~GPUMesh() {
    wgpuBufferRelease(vertices);
    wgpuBufferRelease(indices);
}

WGPUBuffer glengine::pipeline::wgpu::GPUMesh::GetVertices() const {
    return vertices;
}

WGPUBuffer glengine::pipeline::wgpu::GPUMesh::GetIndices() const
{
    return indices;
}