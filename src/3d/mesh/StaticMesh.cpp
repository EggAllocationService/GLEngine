//
// Created by Kyle Smith on 2025-10-21.
//

#include "3d/mesh/StaticMesh.h"
#include <string>

#include "engine_GLUT.h"
using namespace glengine::world::mesh;

std::unique_ptr<StaticMesh> StaticMesh::FromOBJ(std::ifstream& file) {
    auto result = new StaticMesh();

    int vPos = 0, vtPos = 0, vnPos = 0;

    std::string line;
    while (!std::getline(file, line).eof()) {
        if (line.starts_with("v ")) {
            // vertex position
            float x, y, z;
            sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);

            if (vPos >= result->vertices_.size()) {
                result->vertices_.resize(vPos + 1);
            }

            result->vertices_[vPos++].position = float3(x, y, z);
        } else if (line.starts_with("vt ")) {
            // vertex texture coordinate
            float x, y;
            sscanf(line.c_str(), "vt %f %f", &x, &y);

            if (vtPos >= result->vertices_.size()) {
                result->vertices_.resize(vtPos + 1);
            }

            result->vertices_[vtPos++].texCoord = float2(x, y);
            result->hasTexCoords_ = true;
        } else if (line.starts_with("vn ")) {
            // vertex normal
            float x, y, z;
            sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z);

            if (vnPos >= result->vertices_.size()) {
                result->vertices_.resize(vnPos + 1);
            }

            result->vertices_[vnPos++].normal = float3(x, y, z);
            result->hasNormals_ = true;
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
                result->faces_.push_back(int3(p1 - 1, t1 - 1, n1 - 1));
                result->faces_.push_back(int3(p2 - 1, t2 - 1, n2 - 1));
                result->faces_.push_back(int3(p3 - 1, t3 - 1, n3 - 1));
            } else {
                // this OBJ file just specifies position
                int p1, p2, p3;

                sscanf(line.c_str(), "f %d %d %d", &p1, &p2, &p3);

                result->faces_.push_back(int3(p1 - 1, p2 - 1, p3 - 1));
            }
        }
    }

    return std::unique_ptr<StaticMesh>(result);
}

void StaticMesh::Render() {
    glBegin(GL_TRIANGLES);
    float current = 0;
    float max = 0;

    if (hasTexCoords_ || hasNormals_) {
        max = faces_.size();
        // faces array is one vertex per index, with position index, texcood index, and normal index in each int3
        for (auto vertex : faces_) {
            float color = current / max;
            color *= 0.5;
            color += 0.5;
            glColor3f(color, color, color);
            if (hasTexCoords_) {
                glTexCoord2fv(vertices_[vertex.y].texCoord);
            }
            if (hasNormals_) {
                glNormal3fv(vertices_[vertex.z].normal);
            }
            glVertex3fv(vertices_[vertex.x].position);
            current++;
        }
    } else {
        max = faces_.size() / 3.0;
        // faces array is one face per index, p1, p2, p3
        for (auto face : faces_) {
            float color = current / max;
            color *= 0.5;
            color += 0.5;
            glColor3fv(float3(color, color, color));
            glVertex3fv(vertices_[face.x].position);
            glVertex3fv(vertices_[face.y].position);
            glVertex3fv(vertices_[face.z].position);
            current++;
        }
    }

    glEnd();
}
