//
// Created by Kyle Smith on 2026-05-28.
//

#include "pipeline/wgpu/GPUMesh.h"

glengine::pipeline::wgpu::GPUMesh::GPUMesh(WGPUBuffer vertices, WGPUBuffer indices, int vertexCount, int indexCount, int id) {
    this->vertices = vertices;
    this->indices = indices;
    this->vertexCount = vertexCount;
    this->indexCount = indexCount;
    this->id = id;
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