#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "meshoptimizer.h"
#include "../include/Vectors.h"

struct PakHeader {
    char magic[4];
    int version;
    int entryCount;
};

struct MeshHeader {
    char magic[4];
    size_t meshletsSize;
    size_t verticesSize;
    size_t meshletVerticesSize;
    size_t meshletIndicesSize;
};
struct MeshVertex {
    alignas(16) float3 position;
    alignas(16) float3 normal;
    float2 uv;

    bool operator==(const MeshVertex& other) const {
        return memcmp(this, &other, sizeof(MeshVertex)) == 0;
    }
};

struct alignas(64) Meshlet {
    alignas(16) float3 origin;
    alignas(16) float3 coneApex;
    alignas(16) float3 coneAxis;
    unsigned int verticesOffset;
    unsigned int verticesSize;
    unsigned int indicesOffset;
    unsigned int indicesSize;
};

namespace std {
    template <>
    struct hash<MeshVertex> {
        size_t operator()(const MeshVertex& vertex) const noexcept {
            size_t h1 = hash<float>()(vertex.position[0]) ^ hash<float>()(vertex.position[1]) ^ hash<float>()(vertex.position[2]);
            size_t h2 = hash<float>()(vertex.normal[0]) ^ hash<float>()(vertex.normal[1]) ^ hash<float>()(vertex.normal[2]);
            size_t h3 = hash<float>()(vertex.uv[0]) ^ hash<float>()(vertex.uv[1]);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

std::string optimizeMesh(const char* fileName) {
    tinyobj::ObjReaderConfig config;
    config.triangulation_method = "earcut";
    config.vertex_color = false;
    config.triangulate = true;

    std::cout << "Optimizing " << fileName << std::endl;
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(fileName, config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }
    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& shapes = reader.GetShapes();
    auto& attrib = reader.GetAttrib();
    std::vector<MeshVertex> vertices;
    std::vector<unsigned int> indices;
    std::unordered_map<MeshVertex, uint32_t> vertex_indices{};

    for (auto& shape : shapes) {
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f += 3) {
            for (int i = 0; i < 3; ++i) {
                MeshVertex vertex;
                tinyobj::index_t idx = shape.mesh.indices[f + i];

                vertex.position = *reinterpret_cast<const float3*>(&attrib.vertices[3 * size_t(idx.vertex_index)]);
                if (idx.normal_index >= 0) {
                    vertex.normal = *reinterpret_cast<const float3*>(&attrib.normals[3 * size_t(idx.normal_index)]);
                }

                if (idx.texcoord_index >= 0) {
                    vertex.uv = *reinterpret_cast<const float2*>(&attrib.texcoords[2 * size_t(idx.texcoord_index)]);
                }

                if (vertex_indices.count(vertex) == 0) {
                    // add vertex to flattened output
                    vertex_indices[vertex] = uint32_t(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(vertex_indices[vertex]);
            }
        }
    }

    // perform optimization passes
    meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertices.size());
    meshopt_optimizeOverdraw(indices.data(), indices.data(), indices.size(), &vertices[0].position.x, vertices.size(), sizeof(MeshVertex), 1.05f);


    // generate meshlets
    const size_t max_vertices = 64;
    const size_t max_triangles = 96;
    const float cone_weight = 0.25;

    size_t maxMeshlets = meshopt_buildMeshletsBound(indices.size(), max_vertices, max_triangles);
    std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
    std::vector<unsigned int> meshlet_vertices(indices.size());
    std::vector<unsigned char> meshlet_indices(indices.size());


    auto meshletCount = meshopt_buildMeshlets(
        meshlets.data(),
        meshlet_vertices.data(),
        meshlet_indices.data(),
        indices.data(),
        indices.size(),
        &vertices[0].position.x,
        vertices.size(),
        sizeof(MeshVertex),
        max_vertices,
        max_triangles,
        cone_weight
        );
    meshlets.resize(meshletCount);
    const meshopt_Meshlet& last = meshlets[meshletCount - 1];
    meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
    meshlet_indices.resize(last.triangle_offset + last.triangle_count * 3);


    std::ostringstream output;
    MeshHeader header = {
        {'G', 'M', 'S', 'H'},
        meshletCount * sizeof(Meshlet),
        vertices.size() * sizeof(MeshVertex),
        meshlet_vertices.size() * sizeof(unsigned int),
        meshlet_indices.size() * sizeof(char)
    };

    output.write(reinterpret_cast<const std::ostream::char_type *>(&header), sizeof(header));

    // write meshlets
    for (auto& meshlet : meshlets) {
        auto bounds = meshopt_computeMeshletBounds(
            &meshlet_vertices[meshlet.vertex_offset],
            &meshlet_indices[meshlet.triangle_offset],
            meshlet.triangle_count,
            &vertices[0].position.x,
            vertices.size(),
            sizeof(MeshVertex)
        );
        auto result = Meshlet {
            .origin = float3(bounds.center[0], bounds.center[1], bounds.center[2]),
            .coneApex = float3(bounds.cone_apex[0], bounds.cone_apex[1], bounds.cone_apex[2]),
            .coneAxis = float3(bounds.cone_axis[0], bounds.cone_axis[1], bounds.cone_axis[2]),
            .verticesOffset = meshlet.vertex_offset,
            .verticesSize = meshlet.vertex_count,
            .indicesOffset = meshlet.triangle_offset,
            .indicesSize = meshlet.triangle_count
        };
        output.write(reinterpret_cast<const std::ostream::char_type *>(&result), sizeof(result));
    }

    // write vertices
    output.write(reinterpret_cast<char*>(vertices.data()), vertices.size() * sizeof(MeshVertex));

    // write meshlet vertices
    output.write(reinterpret_cast<char*>(meshlet_vertices.data()), meshlet_vertices.size() * sizeof(unsigned int));

    // write meshlet indices
    output.write(reinterpret_cast<char*>(meshlet_indices.data()), indices.size() * sizeof(char));

    return output.str();
}



int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <output.pak> <input1> <input2> ... <name1> <name2> ...\n";
        return 1;
    }

    const char* outputFile = argv[1];
    int remaining = argc - 2;
    if (remaining % 2 != 0) {
        std::cerr << "Error: number of input files and names must match.\n";
        return 1;
    }
    int entryCount = remaining / 2;

    std::ofstream out(outputFile, std::ios::binary);
    if (!out) {
        std::cerr << "Error: cannot open output file: " << outputFile << "\n";
        return 1;
    }

    PakHeader header;
    header.magic[0] = 'G';
    header.magic[1] = 'P';
    header.magic[2] = 'A';
    header.magic[3] = 'K';
    header.version = 1;
    header.entryCount = entryCount;

    out.write(header.magic, sizeof(header.magic));
    out.write(reinterpret_cast<const char*>(&header.version), sizeof(header.version));
    out.write(reinterpret_cast<const char*>(&header.entryCount), sizeof(header.entryCount));

    for (int i = 0; i < entryCount; ++i) {
        const char* inputPath = argv[2 + i];
        const char* name = argv[2 + entryCount + i];
        std::string nameStr(name);
        if (!nameStr.empty() && nameStr[0] != '/') {
            nameStr = '/' + nameStr;
        }
        unsigned short nameLength = static_cast<unsigned short>(nameStr.size());
        out.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        out.write(nameStr.data(), nameLength);

       if (std::string_view(inputPath).ends_with(".obj") && nameStr.ends_with(".mesh")) {
          // optimize mesh for meshlet rendering
           auto data = optimizeMesh(inputPath);
           auto dataLength = static_cast<unsigned int>(data.size());
           out.write(reinterpret_cast<const char*>(&dataLength), sizeof(dataLength));
           out.write(data.data(), dataLength);
       } else {
           std::ifstream in(inputPath, std::ios::binary | std::ios::ate);
           if (!in) {
               std::cerr << "Error: cannot open input file: " << inputPath << "\n";
               return 1;
           }
           auto size = in.tellg();
           in.seekg(0, std::ios::beg);

           std::vector<char> buffer(static_cast<size_t>(size));
           if (!in.read(buffer.data(), size)) {
               std::cerr << "Error: cannot read input file: " << inputPath << "\n";
               return 1;
           }
           unsigned int dataLength = static_cast<unsigned int>(size);

           out.write(reinterpret_cast<const char*>(&dataLength), sizeof(dataLength));
           out.write(buffer.data(), dataLength);
       }
    }

    out.close();
    std::cout << "Created " << outputFile << " with " << entryCount << " entries.\n";
    return 0;
}
