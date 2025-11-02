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
                vertices_.resize((vPos + 1) * 2);
            }

            vertices_[vPos++].position = float3(x, y, z);
        } else if (line.starts_with("vt ")) {
            // vertex texture coordinate
            float x, y;
            sscanf(line.c_str(), "vt %f %f", &x, &y);

            if (vtPos >= vertices_.size()) {
                vertices_.resize((vtPos + 1) * 2);
            }

            vertices_[vtPos++].texCoord = float2(x, y);
            hasTexCoords_ = true;
        } else if (line.starts_with("vn ")) {
            // vertex normal
            float x, y, z;
            sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z);

            if (vnPos >= vertices_.size()) {
                vertices_.resize((vnPos + 1) * 2);
            }

            vertices_[vnPos++].normal = float3(x, y, z);
            hasNormals_ = true;
        } else if (line.starts_with("f ")) {
            // face data
            int p1, p2, p3;
            int t1, t2, t3;
            int n1, n2, n3;

            if (line.find("//") != std::string::npos) {
                // specifying position and normnal, but no u
                sscanf(line.c_str(), "f %d//%d %d//%d %d//%d", &p1, &n1, &p2, &n2, &p3, &n3);

                t1 = t2 = t3 = 0;
            } else if (line.find('/') != std::string::npos) {
                // this OBJ file is specifying pos, normal, and uv
                sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &p1, &t1, &n1,
                                                                     &p2, &t2, &n2,
                                                                     &p3, &t3, &n3);
            } else {
                // this OBJ file just specifies position

                sscanf(line.c_str(), "f %d %d %d", &p1, &p2, &p3);

                // we'll say that the normal for vertex n is also stored with the position in this case
                // is even any use for reusing the same vertex with different normals
                n1 = p1;
                n2 = p2;
                n3 = p3;

                // just say all the texCoords are at vertex 0 (1-indexed :why:)
                t1 = 1;
                t2 = 1;
                t3 = 1;
               
            }

            faces_.push_back(int3(p1 - 1, t1 - 1, n1 - 1));
            faces_.push_back(int3(p2 - 1, t2 - 1, n2 - 1));
            faces_.push_back(int3(p3 - 1, t3 - 1, n3 - 1));
        }
    }

    normalize();
    if (!hasNormals_) {
        RecalculateNormals();
    }
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

struct AvgNormal {
    float3 accumulator = float3(0, 0, 0);
    float totalLen = 0.0;
};

void glengine::world::mesh::StaticMesh::RecalculateNormals()
{
    // absolutely horrible
    // however, any algorithm that works is a good algorithm
    // if it completes before the heat death of the universe then it's a great algorithm
    auto newNormals = new AvgNormal[vertices_.size()];


    // loop through all faces, calculate the normal, then add to the average for each owning vertex
    // faces are packed in the faces_ array, three entries in a row make a face
    // each entry is an int3(vertex, texCoord, normal) tuple
    for (int i = 0; i < faces_.size(); i += 3) {
        auto& v1 = faces_[i];
        auto& v2 = faces_[i + 1];
        auto& v3 = faces_[i + 2];

        auto p1 = vertices_[v1.x].position;
        auto p2 = vertices_[v2.x].position;
        auto p3 = vertices_[v3.x].position;

        // calculate normal from vertex positions
        auto normal = (p1 - p2).cross(p3 - p2) * -1;

        // add normal to each vertex's average, along with its length
        auto len = normal.len();
        newNormals[v1.x].accumulator.xyz += normal;
        newNormals[v1.x].totalLen += len;

        newNormals[v2.x].accumulator.xyz += normal;
        newNormals[v2.x].totalLen += len;

        newNormals[v3.x].accumulator.xyz += normal;
        newNormals[v3.x].totalLen += len;

        // for each point in the face, update its normal index to point to the same vertex as the position
        v1.z = v1.x;
        v2.z = v2.x;
        v3.z = v3.x;
    }

    // now that we've calculated a running total, we can use that to find the average normal for each vertex in the mesh
    for (int i = 0; i < vertices_.size(); i++) {
        auto normal = (newNormals[i].accumulator / newNormals[i].totalLen).norm();

        vertices_[i].normal = normal;
    }

    // indicate that we now have normals! woohoo!
    hasNormals_ = true;

    // cleanup
    delete[] newNormals;
}

void StaticMesh::Render() const {
    glBegin(GL_TRIANGLES);

    // faces array is one vertex per index, with position index, texcoord index, and normal index in each int3
    for (auto vertex : faces_) {
        if (hasTexCoords_) {
            glTexCoord2fv(vertices_[vertex.y].texCoord);
        }

        // guarenteed to have normals, since we calculate if they weren't in the file
        glNormal3fv(vertices_[vertex.z].normal);
        glVertex3fv(vertices_[vertex.x].position);
    }

    glEnd();
}
