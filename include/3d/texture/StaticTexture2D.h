//
// Created by Kyle Smith on 2025-11-19.
//
#pragma once

#include "glengine_export.h"
#include "Resource.h"

namespace glengine::world::texture {
    class GLENGINE_EXPORT StaticTexture2D : public Resource {
    public:
        void LoadFromFile(std::ifstream &) override;
        void Bind() const;
        void SetParameter(int param, int value);
    private:
        unsigned int textureID = -1;
    };
}
