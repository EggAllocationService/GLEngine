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
        auto layout = engine->GetRenderObjectsManager()->GetObject<objects::OptimizedDrawTracker>()->GetPipeline()->GetBindGroupLayout(1);

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

        auto session = renderer->GetTransferManager()->CreateSession(std::format("OptMesh {} upload", id), header.meshletIndicesSize + header.meshletVerticesSize + header.meshletsSize);
        char* mesh_data = new char[header.meshletsSize];
        char* vert_data = new char[header.meshletVerticesSize];
        char* ind_data = new char[header.meshletIndicesSize];

        file.read(mesh_data, header.meshletsSize);
        file.read(vert_data, header.meshletVerticesSize);
        file.read(ind_data, header.meshletIndicesSize);

        session->Transfer(meshlets, 0, mesh_data, header.meshletsSize);
        session->Transfer(vertices, 0, vert_data, header.meshletVerticesSize);
        session->Transfer(indices, 0, ind_data, header.meshletIndicesSize);

        session->Commit();

        delete[] mesh_data;
        delete[] vert_data;
        delete[] ind_data;

        auto entries = new WGPUBindGroupEntry[3] {WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT};
        entries[0].buffer = meshlets;
        entries[1].buffer = vertices;
        entries[1].binding = 1;
        entries[2].buffer = indices;
        entries[2].binding = 2;

        WGPUBindGroupDescriptor desc = {
            .nextInChain = nullptr,
            .label = {nullptr, 0},
            .layout = layout,
            .entryCount = 3,
            .entries = entries
        };
        group = wgpuDeviceCreateBindGroup(renderer->GetDevice(), &desc);

        delete[] entries;

        meshletCount = header.meshletCount;
    }

    OptimizedMesh::~OptimizedMesh() {
        wgpuBindGroupRelease(group);
    }

    int OptimizedMesh::GetId() const {
        return id;
    }

    WGPUBindGroup OptimizedMesh::GetBindGroup() {
        return group;
    }
}
