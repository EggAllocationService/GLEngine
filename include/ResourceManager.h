//
// Created by Kyle Smith on 2025-10-27.
//
#pragma once
#include <string>

#include "glengine_export.h"
#include "Resource.h"

namespace glengine {
    class GLENGINE_EXPORT ResourceManager {
    public:
        ResourceManager(pipeline::wgpu::WGPURenderer* renderer) {
            this->renderer = renderer;
        }

        /// Gets or loads a resource from the given file name
        template <typename T>
        std::shared_ptr<T> GetResource(std::string_view path) {
            static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
            static_assert(std::is_constructible_v<T, std::istream&, pipeline::wgpu::WGPURenderer*>, "T must have a standard resource constructor");

            const auto name = std::string(path);

            const auto found = resources.find(name);
            if (found == resources.end()) {
                // create then add to map

                std::ifstream file;
                file.open(name.c_str(), std::ios::binary);

                auto resource = std::make_shared<T>(file, renderer);
                resources[name] = resource;
                return resource;
            }

            return std::dynamic_pointer_cast<T>(found->second);
        }

        template<typename T>
        std::shared_ptr<T> GetResource(std::string_view path, const char* data, size_t len) {
            static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
            static_assert(std::is_constructible_v<T, std::istream&, pipeline::wgpu::WGPURenderer*>, "T must have a standard resource constructor");
            const auto name = std::string(path);

            const auto found = resources.find(name);
            if (found == resources.end()) {
                // create then add to map
                auto stream = CreateStreamBuffer(data, len);

                auto resource = std::make_shared<T>(*stream, renderer);
                resources[name] = resource;
                return resource;
            }

            return std::dynamic_pointer_cast<T>(found->second);
        }

        static std::unique_ptr<std::istream> CreateStreamBuffer(const char* data, size_t len);
    private:
        pipeline::wgpu::WGPURenderer *renderer;
        std::unordered_map<std::string, std::shared_ptr<Resource>> resources;
    };
}
