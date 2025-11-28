//
// Created by Kyle Smith on 2025-11-19.
//
#include "3d/texture/StaticTexture2D.h"
#include <util/stb_image.h>
#include "engine_GLUT.h"
#include <memory>

using namespace glengine::world::texture;

void StaticTexture2D::LoadFromFile(std::ifstream &stream) {
    // figure out how big the image file is
    stream.seekg(0, std::ios::end);
    auto size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    // allocate a buffer and read file contents
    auto buffer = std::make_unique<char[]>(size);
    stream.read(buffer.get(), size);
    stream.close();

    int x, y, n;
    auto decoded = stbi_load_from_memory((unsigned char*) buffer.get(), size, &x, &y, &n, 4);

    // allocate texture
    if (textureID != -1) {
        glDeleteTextures(1, &textureID);
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGetError();
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, x, y, GL_RGBA, GL_UNSIGNED_BYTE, decoded);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(decoded);
}

void StaticTexture2D::Bind() const {
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void StaticTexture2D::SetParameter(int param, int value) {
    Bind();
    glTexParameteri(GL_TEXTURE_2D, param, value);
    glBindTexture(GL_TEXTURE_2D, 0);
}
