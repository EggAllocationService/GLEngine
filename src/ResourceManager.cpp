//
// Created by Kyle Smith on 2026-06-01.
//
#include "ResourceManager.h"
#include "PakFile.h"

#include <streambuf>
#include <istream>
#include <span>
#include <cstddef>
#include <iostream>
#include <vector>

#include "3d/mesh/StaticMesh.h"
#include "3d/texture/StaticTexture2D.h"

#ifdef GLENGINE_TEXT_RENDERING
#include "3d/text/Font.h"
#endif

// 1. The Stream Buffer: Handles memory mapping and seeking
class imemstream_buf : public std::streambuf {
public:
    imemstream_buf(const char* base, size_t size) {
        // std::streambuf requires non-const pointers for historical reasons.
        // It is perfectly safe because we never allow output/writing operations.
        char* p = const_cast<char*>(base);

        // setg sets the "get" area pointers: begin, current, end
        this->setg(p, p, p + size);
    }

protected:
    // Override seekoff to support seekg(offset, direction)
    pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                     std::ios_base::openmode which = std::ios_base::in) override {

        // We only support input operations
        if (!(which & std::ios_base::in)) {
            return pos_type(off_type(-1));
        }

        char* curr = this->gptr();
        char* begin = this->eback();
        char* end = this->egptr();
        char* next = nullptr;

        switch (dir) {
            case std::ios_base::beg: next = begin + off; break;
            case std::ios_base::cur: next = curr + off; break;
            case std::ios_base::end: next = end + off; break;
            default: return pos_type(off_type(-1));
        }

        // Prevent seeking out of bounds
        if (next < begin || next > end) {
            return pos_type(off_type(-1));
        }

        // Update the current get pointer
        this->setg(begin, next, end);

        // Return the new absolute position
        return pos_type(next - begin);
    }

    // Override seekpos to support seekg(absolute_position) and tellg()
    pos_type seekpos(pos_type sp, std::ios_base::openmode which = std::ios_base::in) override {
        return seekoff(off_type(sp), std::ios_base::beg, which);
    }
};

// 2. The Input Stream: Provides the std::istream interface
class imemstream : public std::istream {
private:
    imemstream_buf buffer;

public:
    // Base constructor taking raw pointer and length
    imemstream(const char* base, size_t size)
        : std::istream(nullptr), buffer(base, size) {
        this->rdbuf(&buffer); // Bind the base istream to our custom buffer
    }

    // C++20 constructor for std::span<const char>
    explicit imemstream(std::span<const char> data)
        : imemstream(data.data(), data.size()) {}

    // C++20 constructor for std::span<const std::byte> (raw binary)
    explicit imemstream(std::span<const std::byte> data)
        : imemstream(reinterpret_cast<const char*>(data.data()), data.size()) {}

    // C++20 constructor for std::span<const uint8_t>
    explicit imemstream(std::span<const uint8_t> data)
        : imemstream(reinterpret_cast<const char*>(data.data()), data.size()) {}
};

template<>
std::shared_ptr<WGPUShaderModule> glengine::internal::ConstructResource<WGPUShaderModule>(std::istream &stream,
    pipeline::wgpu::WGPURenderer *renderer) {
    // figure out how big the image file is
    stream.seekg(0, std::ios::end);
    auto size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    // allocate a buffer and read file contents
    auto buffer = new char[static_cast<unsigned long>(size) + 1];
    stream.read(buffer, size);
    buffer[size] = '\0';

    return std::make_shared<WGPUShaderModule>(renderer->CompileShader(buffer));
}

#include <iostream>
glengine::ResourceManager::ResourceManager(pipeline::wgpu::WGPURenderer *renderer) {
    this->renderer = renderer;

    RegisterResourceType<world::mesh::StaticMesh>();
    RegisterResourceType<world::texture::StaticTexture2D>();

#ifdef GLENGINE_TEXT_RENDERING
    RegisterResourceType<world::font::Font>();
#endif
}

void glengine::ResourceManager::MountPak(std::string_view path, std::string_view fileName) {
    std::ifstream file;
    file.open(fileName);
    if (file.is_open()) {
        MountPak(path, file);
    } else {
        std::cerr << "MountPak: failed to open file\n";
    }
}

void glengine::ResourceManager::MountPak(std::string_view path, std::istream &data) {
    pak::PakHeader header;
    data.read(header.magic, sizeof(header.magic));
    data.read(reinterpret_cast<char*>(&header.version), sizeof(header.version));
    data.read(reinterpret_cast<char*>(&header.entryCount), sizeof(header.entryCount));

    if (header.magic[0] != 'G' || header.magic[1] != 'P' ||
        header.magic[2] != 'A' || header.magic[3] != 'K') {
        std::cerr << "MountPak: invalid magic\n";
        return;
    }

    if (header.version != 1) {
        std::cerr << "MountPak: unsupported version " << header.version << "\n";
        return;
    }

    std::string prefix(path);
    while (!prefix.empty() && prefix.back() == '/') {
        prefix.pop_back();
    }

    for (int i = 0; i < header.entryCount; ++i) {
        unsigned short nameLength = 0;
        data.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));

        std::string name(nameLength, '\0');
        data.read(name.data(), nameLength);

        unsigned int length = 0;
        data.read(reinterpret_cast<char*>(&length), sizeof(length));

        auto buffer = new char[length];
        data.read(buffer, length);

        Blob blob = {
            .data = buffer,
            .length = length
        };

        std::string key = prefix + name;
        blobs[std::move(key)] = blob;
    }
}

void glengine::ResourceManager::MountPak(std::string_view path, const void *data, size_t len) {
    auto stream = CreateStreamBuffer(static_cast<const char *>(data), len);
    MountPak(path, *stream);
}

std::unique_ptr<std::istream> glengine::ResourceManager::CreateStreamBuffer(const char *data, size_t len) {
    return std::make_unique<imemstream>(data, len);
}

std::unique_ptr<std::istream> glengine::ResourceManager::OpenResource(std::string_view path) {
    std::string pathName(path);

    if (blobs.contains(pathName)) {
        // we mounted a pak file with this resource in it
        const auto& blob = blobs[pathName];
        return CreateStreamBuffer(blob.data, blob.length);
    } else {
        auto file = std::make_unique<std::ifstream>();
        file->open(pathName);
        return file;
    }
}
