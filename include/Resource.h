//
// Created by Kyle Smith on 2025-10-27.
//
#pragma once
#include <fstream>

#include "glengine_export.h"

namespace glengine {
    class GLENGINE_EXPORT Resource {
    public:
        virtual ~Resource() = default;

        virtual void LoadFromFile(std::ifstream&) = 0;
    };
}