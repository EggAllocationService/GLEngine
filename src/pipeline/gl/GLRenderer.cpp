//
// Created by Kyle Smith on 2025-11-26.
//

#include "pipeline/gl/GLRenderer.h"
using namespace glengine::world::mesh;
using namespace glengine::pipeline::gl;

GLRenderer::GLRenderer() {
    // todo methinks
}

void GLRenderer::SetMatrices(mat4 viewMatrix, mat4 projectionMatrix) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&projectionMatrix.data[0]);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&viewMatrix.data[0]);
}

void GLRenderer::RenderMesh(const StaticMesh *mesh, Material material, mat4 transform) {
    material.Load();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(&transform.data[0]);

    glBegin(GL_TRIANGLES);
    for (const auto face : mesh->faces_) {
        glTexCoord2fv(mesh->vertices_[face.y].texCoord);
        glNormal3fv(mesh->vertices_[face.y].normal);
        glVertex3fv(mesh->vertices_[face.x].position);
    }
    glEnd();

    glPopMatrix();
}
