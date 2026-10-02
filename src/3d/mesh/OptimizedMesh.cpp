//
// Created by Kyle Smith on 2026-10-01.
//

#include "3d/mesh/OptimizedMesh.h"

#include "Engine.h"
#include <cstring>
#include <zstd.h>

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

        // load file into memory to decompress
        file.seekg(0, std::ios::end);
        auto len = file.tellg();
        file.seekg(0, std::ios::beg);

        auto compressed = new char[len];
        file.read(compressed, len);

        auto decompressedSize = ZSTD_getFrameContentSize(compressed, len);

        std::string decompressed;
        decompressed.resize(decompressedSize);
        ZSTD_decompress(decompressed.data(), decompressedSize, compressed, len);

        delete[] compressed;

        // first, load header
        auto header = *reinterpret_cast<MeshHeader*>(decompressed.data());
        if (memcmp(header.magic, "GMSH", 4) != 0) {
            std::cerr << "Invalid mesh data" << std::endl;
            return;
        }
        id = LastId++;

        meshlets = renderer->CreateRawBuffer(std::format("OptMesh {} meshlet data", id), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage, header.meshletsSize);
        vertices = renderer->CreateRawBuffer(std::format("OptMesh {} vertex data", id), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage, header.meshletVerticesSize);
        indices = renderer->CreateRawBuffer(std::format("OptMesh {} index data", id), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage, header.meshletIndicesSize);

        auto queue = wgpuDeviceGetQueue(renderer->GetDevice());

        auto meshPtr = decompressed.data() + sizeof(MeshHeader);
        auto vertPtr = meshPtr + header.meshletsSize;
        auto indPtr = vertPtr + header.meshletVerticesSize;

        wgpuQueueWriteBuffer(queue,meshlets, 0, meshPtr, header.meshletsSize);
        wgpuQueueWriteBuffer(queue,vertices, 0, vertPtr, header.meshletVerticesSize);
        wgpuQueueWriteBuffer(queue,indices, 0, indPtr, header.meshletIndicesSize);

        meshletCount = header.meshletCount;
    }

    OptimizedMesh::~OptimizedMesh() {
        wgpuBindGroupRelease(group);
    }

    int OptimizedMesh::GetId() const {
        return id;
    }
}
