//
// Created by Kyle Smith on 2025-10-27.
//
#pragma once
#include <string>

#include "glengine_export.h"
#include "Resource.h"

namespace glengine {
    namespace internal {
        template<typename T>
        std::shared_ptr<T> ConstructResource(std::istream& stream, pipeline::wgpu::WGPURenderer* renderer) {
            return std::make_shared<T>(stream, renderer);
        }

        template<>
        GLENGINE_EXPORT std::shared_ptr<WGPUShaderModule> ConstructResource<WGPUShaderModule>(std::istream& stream, pipeline::wgpu::WGPURenderer* renderer);

        template <typename T>
consteval std::string_view GetTypeName() {
#if defined(__clang__)
            constexpr std::string_view name = __PRETTY_FUNCTION__;
            // Clang format: std::string_view GetTypeName() [T = int]
            constexpr std::string_view prefix = "T = ";
            constexpr std::string_view suffix = "]";
#elif defined(__GNUC__)
            constexpr std::string_view name = __PRETTY_FUNCTION__;
            // GCC format: consteval std::string_view GetTypeName() [with T = int]
            constexpr std::string_view prefix = "with T = ";
            constexpr std::string_view suffix = "]";
#elif defined(_MSC_VER)
            constexpr std::string_view name = __FUNCSIG__;
            // MSVC format: class std::basic_string_view<...> __cdecl GetTypeName<int>(void)
            constexpr std::string_view prefix = "GetTypeName<";
            constexpr std::string_view suffix = ">(void)";
#else
#error "Unsupported compiler!"
#endif

            // Locate the start of the type name
            constexpr std::size_t start_pos = name.find(prefix);
            static_assert(start_pos != std::string_view::npos, "Compiler macro format changed or unsupported.");

            constexpr std::size_t start = start_pos + prefix.size();

            // Locate the end of the type name
            constexpr std::size_t end = name.rfind(suffix);
            static_assert(end != std::string_view::npos && end >= start, "Compiler macro format changed or unsupported.");

            // Return the sliced view
            return name.substr(start, end - start);
        }
    }

    struct Blob {
        char* data;
        size_t length;
    };
    class GLENGINE_EXPORT ResourceManager {
    public:
        ResourceManager(pipeline::wgpu::WGPURenderer* renderer);

        /// Gets or loads a resource from the given file name
        template <typename T>
        std::shared_ptr<T> GetResource(std::string_view path) {
            static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
            static_assert(std::is_constructible_v<T, std::istream&, pipeline::wgpu::WGPURenderer*>, "T must have a standard resource constructor");

            const auto name = std::string(path);

            const auto found = resources.find(name);
            if (found == resources.end()) {
                // create then add to map
                auto data = OpenResource(path);
                auto resource = internal::ConstructResource<T>(*data, renderer);
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
                auto resource = internal::ConstructResource<T>(*stream, renderer);
                resources[name] = resource;
                return resource;
            }

            return std::dynamic_pointer_cast<T>(found->second);
        }

        template<typename T>
        void RegisterResourceType() {
            static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
            static_assert(std::is_constructible_v<T, std::istream&, pipeline::wgpu::WGPURenderer*>, "T must have a standard resource constructor");

            std::string name(internal::GetTypeName<T>());
            registeredConstructors[std::move(name)] = [](std::istream& stream, pipeline::wgpu::WGPURenderer* renderer) {
                return std::dynamic_pointer_cast<Resource>(internal::ConstructResource<T>(stream, renderer));
            };
        }

        void MountPak(std::string_view path, std::string_view fileName);
        void MountPak(std::string_view path, std::istream& data);
        void MountPak(std::string_view path, const void* data, size_t len);
    private:
        static std::unique_ptr<std::istream> CreateStreamBuffer(const char* data, size_t len);
        std::unique_ptr<std::istream> OpenResource(std::string_view path);
        pipeline::wgpu::WGPURenderer *renderer;
        std::unordered_map<std::string, std::shared_ptr<Resource>> resources;
        std::unordered_map<std::string, std::function<std::shared_ptr<Resource>(std::istream&, pipeline::wgpu::WGPURenderer*)>> registeredConstructors;
        std::unordered_map<std::string, Blob> blobs;
    };
}
