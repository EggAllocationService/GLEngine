//
// Created by Kyle Smith on 2025-10-21.
//

#include "3d/mesh/StaticMesh.h"
#include <string>

#include "engine_GLUT.h"
using namespace glengine::world::mesh;

void StaticMesh::LoadFromFile(std::ifstream &file) {
    vertices_.clear();
    faces_.clear();

    int vPos = 0, vtPos = 0, vnPos = 0;

    std::string line;
    while (!std::getline(file, line).eof()) {
        if (line.starts_with("v ")) {
            // vertex position
            float x, y, z;
            sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);

            if (vPos >= vertices_.size()) {
                vertices_.resize(vPos + 1);
            }

            vertices_[vPos++].position = float3(x, y, z);
        } else if (line.starts_with("vt ")) {
            // vertex texture coordinate
            float x, y;
            sscanf(line.c_str(), "vt %f %f", &x, &y);

            if (vtPos >= vertices_.size()) {
                vertices_.resize(vtPos + 1);
            }

            vertices_[vtPos++].texCoord = float2(x, y);
            hasTexCoords_ = true;
        } else if (line.starts_with("vn ")) {
            // vertex normal
            float x, y, z;
            sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z);

            if (vnPos >= vertices_.size()) {
                vertices_.resize(vnPos + 1);
            }

            vertices_[vnPos++].normal = float3(x, y, z);
            hasNormals_ = true;
        } else if (line.starts_with("f ")) {
            // face data
            if (line.find('/') != std::string::npos) {
                // this OBJ file is specifying pos, normal, and uv
                int p1, p2, p3;
                int t1, t2, t3;
                int n1, n2, n3;
                sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &p1, &t1, &n1,
                                                                     &p2, &t2, &n2,
                                                                     &p3, &t3, &n3);
                faces_.push_back(int3(p1 - 1, t1 - 1, n1 - 1));
                faces_.push_back(int3(p2 - 1, t2 - 1, n2 - 1));
                faces_.push_back(int3(p3 - 1, t3 - 1, n3 - 1));
            } else {
                // this OBJ file just specifies position
                int p1, p2, p3;

                sscanf(line.c_str(), "f %d %d %d", &p1, &p2, &p3);

                faces_.push_back(int3(p1 - 1, p2 - 1, p3 - 1));
            }
        }
    }

    normalize();
    uploadToGPU();
}

float max(float a, float b, float c, float d) {
    return std::max(std::max(a, std::max(b, c)), d);
}

float3 abs(float3 a) {
    return {std::abs(a.x), std::abs(a.y), std::abs(a.z)};
}

void StaticMesh::normalize() {
    if (vertices_.empty()) {
        return;
    }

    float maxPos = 0;

    // find largest coord (abs)
    for (const auto& vertex :vertices_) {
        auto position = abs(vertex.position);
        maxPos = max(maxPos, position.x, position.y, position.z);
    }

    // divide by largest
    for (auto& vertex : vertices_) {
        vertex.position = vertex.position / maxPos;
    }
}

struct PackedVertex {
    float3 position;
    float3 normal;
    float3 color;

    // this is a float3 for padding reasons
    // only x and y are used
    float3 texCoord;
};

void StaticMesh::uploadToGPU() {
    // first, create a new verted buffer and index buffer
    glGenBuffers(1, &vertexBuffer_);

    // first lets pack all vertex data
    std::vector<PackedVertex> vertices;

    auto n = faces_.size();
    if (!hasNormals_ && !hasTexCoords_) n *= 3;
    vertices.reserve(n);

    float i = 0;

    for (const auto& face : faces_) {
        PackedVertex vertex;
        if (!hasNormals_ && !hasTexCoords_) {
            // face is actually 3 position indices
            vertex.color = float3(i / n, i / n, i / n);
            vertex.position = vertices_[face.x].position;
            vertices.push_back(vertex);

            vertex.position = vertices_[face.y].position;
            vertices.push_back(vertex);

            vertex.position = vertices_[face.z].position;
            vertices.push_back(vertex);

            i += 3;
        } else {
            vertex.position = vertices_[face.x].position;
            vertex.texCoord.xy = vertices_[face.y].texCoord;
            vertex.normal = vertices_[face.z].normal;
            vertex.color = float3(i / n, i / n, i / n);
            vertices.push_back(vertex);
            i += 1;
        }
    }


    // now upload to the GPU
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PackedVertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    uploadedCount = vertices.size();

}

void StaticMesh::Render() const {
    // bind buffer and draw
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
    glVertexPointer(3, GL_FLOAT, sizeof(PackedVertex), nullptr);
    glColorPointer(3, GL_FLOAT, sizeof(PackedVertex), reinterpret_cast<void *>(sizeof(float3) * 2));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, uploadedCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
