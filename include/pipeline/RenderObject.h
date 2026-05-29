//
// Created by Kyle Smith on 2026-05-29.
//
#pragma once
#include "wgpu/WGPURenderer.h"

namespace glengine::pipeline {
    class RenderObject {
    public:
        virtual ~RenderObject() = default;

        virtual void UpdateEnd(double deltaTime) = 0;

        virtual void RenderStart(wgpu::RenderBundle& bundle) = 0;
    };
}
