//
// Created by Kyle Smith on 2026-06-01.
//
#include "ResourceManager.h"


#include <streambuf>
#include <istream>
#include <span>
#include <cstddef>
#include <iostream>
#include <vector>

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

std::unique_ptr<std::istream> glengine::ResourceManager::CreateStreamBuffer(const char *data, size_t len) {
    return std::make_unique<imemstream>(data, len);
}
