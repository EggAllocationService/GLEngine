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
        /// Gets or loads a resource from the given file name
        template <typename T>
        std::shared_ptr<T> GetResource(std::string_view path) {
            static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
            const auto name = std::string(path);

            const auto found = resources.find(name);
            if (found == resources.end()) {
                // create then add to map
                auto resource = std::make_shared<T>();
                resources[name] = resource;

                std::ifstream file;
                file.open(name.c_str(), std::ios::binary);

                resource->LoadFromFile(file);

                return resource;
            }

            return std::dynamic_pointer_cast<T>(found->second);
        }
    private:
        std::unordered_map<std::string, std::shared_ptr<Resource>> resources;
    };
}
