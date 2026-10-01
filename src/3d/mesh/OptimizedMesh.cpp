//
// Created by Kyle Smith on 2026-10-01.
//

#include "3d/mesh/OptimizedMesh.h"

#include "Engine.h"
#include "3d/objects/OptimizedDrawTracker.h"

struct MeshHeader {
    char magic[4];
    size_t meshletCount;
    size_t meshletsSize;
    size_t meshletVerticesSize;
    size_t meshletIndicesSize;
};

namespace glengine::world::mesh {
    int OptimizedMesh::LastId = 1;

    OptimizedMesh::OptimizedMesh(std::istream &file, pipeline::wgpu::WGPURenderer *renderer) {
        auto engine = Engine::GetCurrentEngine();

        // first, load header
        MeshHeader header;
        file.read((char*)&header, sizeof(MeshHeader));
        if (memcmp(header.magic, "GMSH", 4) != 0) {
            std::cerr << "Invalid mesh data" << std::endl;
            return;
        }
        id = LastId++;

        meshlets = renderer->CreateRawBuffer(std::format("OptMesh {} meshlet data", id), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage, header.meshletsSize);
        vertices = renderer->CreateRawBuffer(std::format("OptMesh {} vertex data", id), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage, header.meshletVerticesSize);
        indices = renderer->CreateRawBuffer(std::format("OptMesh {} index data", id), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage, header.meshletIndicesSize);

        char* mesh_data = new char[header.meshletsSize];
        char* vert_data = new char[header.meshletVerticesSize];
        char* ind_data = new char[header.meshletIndicesSize];

        file.read(mesh_data, header.meshletsSize);
        file.read(vert_data, header.meshletVerticesSize);
        file.read(ind_data, header.meshletIndicesSize);

        auto queue = wgpuDeviceGetQueue(renderer->GetDevice());

        wgpuQueueWriteBuffer(queue,meshlets, 0, mesh_data, header.meshletsSize);
        wgpuQueueWriteBuffer(queue,vertices, 0, vert_data, header.meshletVerticesSize);
        wgpuQueueWriteBuffer(queue,indices, 0, ind_data, header.meshletIndicesSize);

        delete[] mesh_data;
        delete[] vert_data;
        delete[] ind_data;

        meshletCount = header.meshletCount;
    }

    OptimizedMesh::~OptimizedMesh() {
        wgpuBindGroupRelease(group);
    }

    int OptimizedMesh::GetId() const {
        return id;
    }
}
