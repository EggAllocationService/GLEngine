//
// Created by Kyle Smith on 2026-10-01.
//
#define TINYOBJLOADER_IMPLEMENTATION
#include <format>

#include "tiny_obj_loader.h"

#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>

#include "../include/Vectors.h"
#include "meshoptimizer.h"
#include "zstd.h"

struct MeshHeader {
    char magic[4];
    size_t meshletCount;
    size_t meshletsSize;
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

struct Meshlet {
    float4 origin;
    float4 coneApex;
    float4 coneAxis;
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

std::vector<float3> computeAllSmoothingNormals(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes) {
    std::vector<float3> accumulated(attrib.vertices.size() / 3, float3(0, 0, 0));

    for (auto& shape : shapes) {
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f += 1) {
            bool missing = false;
            for (int i = 0; i < 3; ++i) {
                if (shape.mesh.indices[(3 * f) + i].normal_index < 0) {
                    missing = true;
                    break;
                }
            }
            if (!missing) {
                continue;
            }

            const float3 p0 = *reinterpret_cast<const float3*>(&attrib.vertices[3 * size_t(shape.mesh.indices[(3 * f) + 0].vertex_index)]);
            const float3 p1 = *reinterpret_cast<const float3*>(&attrib.vertices[3 * size_t(shape.mesh.indices[(3 * f) + 1].vertex_index)]);
            const float3 p2 = *reinterpret_cast<const float3*>(&attrib.vertices[3 * size_t(shape.mesh.indices[(3 * f) + 2].vertex_index)]);
            const float3 faceNormal = (p1 - p0).cross(p2 - p0);

            for (int i = 0; i < 3; ++i) {
                auto& slot = accumulated[shape.mesh.indices[(3 * f) + i].vertex_index];
                slot = slot + faceNormal;
            }
        }
    }

    std::vector<float3> normals(accumulated.size());
    for (size_t i = 0; i < accumulated.size(); ++i) {
        const float len = accumulated[i].len();
        normals[i] = len > 0 ? accumulated[i] / len : float3(0, 1, 0);
    }
    return normals;
}

std::string optimizeMesh(const char* fileName) {
    tinyobj::ObjReaderConfig config;
    config.triangulation_method = "simple";
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

    auto shapes = reader.GetShapes();
    auto attrib = reader.GetAttrib();
    auto computedNormals = computeAllSmoothingNormals(attrib, shapes);

    std::vector<MeshVertex> vertices;
    std::vector<unsigned int> indices;
    std::unordered_map<MeshVertex, uint32_t> vertex_indices{};

    for (auto& shape : shapes) {
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f += 1) {
            for (int i = 0; i < 3; ++i) {
                MeshVertex vertex{};
                tinyobj::index_t idx = shape.mesh.indices[(3 * f) + i];

                vertex.position = *reinterpret_cast<const float3*>(&attrib.vertices[3 * size_t(idx.vertex_index)]);
                if (idx.normal_index >= 0) {
                    vertex.normal = *reinterpret_cast<const float3*>(&attrib.normals[3 * size_t(idx.normal_index)]);
                } else {
                    vertex.normal = computedNormals[size_t(idx.vertex_index)];
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

    std::cout << std::format("Vertices: {}, Triangles: {}", vertices.size(), indices.size() / 3) << std::endl;

    // perform optimization passes
    meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertices.size());
    meshopt_optimizeOverdraw(indices.data(), indices.data(), indices.size(), &vertices[0].position.x, vertices.size(), sizeof(MeshVertex), 1.05f);

    // scale vertices
    float scale = 0;
    for (const auto& vertex : vertices) {
        scale = std::max(scale, std::abs(vertex.position.x));
        scale = std::max(scale, std::abs(vertex.position.y));
        scale = std::max(scale, std::abs(vertex.position.z));
    }

    for (auto& vertex : vertices) {
        vertex.position = vertex.position / scale;
    }

    // generate meshlets
    const size_t max_vertices = 64;
    const size_t max_triangles = 64;
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
    std::vector<unsigned short> meshlet_indices_u16(meshlet_indices.size());
    for (int i = 0; i < meshlet_indices.size(); ++i) {
        meshlet_indices_u16[i] = meshlet_indices[i];
    }
    if (meshlet_indices_u16.size() % 2 == 1) {
        meshlet_indices_u16.push_back(0); // pad to multiple of 4
    }

    std::ostringstream output;
    MeshHeader header = {
        {'G', 'M', 'S', 'H'},
        meshletCount,
        meshletCount * sizeof(Meshlet),
        meshlet_vertices.size() * sizeof(MeshVertex),
        meshlet_indices_u16.size() * sizeof(unsigned short)
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
            .origin = float4(bounds.center[0], bounds.center[1], bounds.center[2], 0.0),
            .coneApex = float4(bounds.cone_apex[0], bounds.cone_apex[1], bounds.cone_apex[2], 0.0),
            .coneAxis = float4(bounds.cone_axis[0], bounds.cone_axis[1], bounds.cone_axis[2], bounds.cone_cutoff),
            .verticesOffset = meshlet.vertex_offset,
            .verticesSize = meshlet.vertex_count,
            .indicesOffset = meshlet.triangle_offset,
            .indicesSize = meshlet.triangle_count * 3
        };
        output.write(reinterpret_cast<const std::ostream::char_type *>(&result), sizeof(result));
    }

    std::vector<MeshVertex> mapped_vertices(meshlet_vertices.size());
    for (size_t i = 0; i < meshlet_vertices.size(); ++i) {
        mapped_vertices[i] = vertices[meshlet_vertices[i]];
    }

    // write meshlet vertices
    output.write(reinterpret_cast<char*>(mapped_vertices.data()), mapped_vertices.size() * sizeof(MeshVertex));

    // write meshlet indices
    output.write(reinterpret_cast<char*>(meshlet_indices_u16.data()), meshlet_indices_u16.size() * sizeof(unsigned short));

    auto result = output.str();

    std::cout << "Compressing... " << std::endl;

    auto compressBound = ZSTD_compressBound(result.size());
    std::string compressed;
    compressed.resize(compressBound);

    auto actualSize = ZSTD_compress(compressed.data(), compressBound, result.data(), result.size(), 18);

    compressed.resize(actualSize);
    return compressed;
}
