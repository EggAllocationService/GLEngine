//
// Created by Kyle Smith on 2026-05-30.
//
#pragma once
#include "pipeline/RenderObject.h"
#include "pipeline/RenderObjects.h"

namespace glengine::world::objects {
    class LightTracker: public pipeline::RenderObject {
    public:
        LightTracker();
        void UpdateEnd(double deltaTime) override;

        void AddLight(rendering::LightInfo info);

        void RenderStart(pipeline::wgpu::RenderBundle &bundle) override;

        int GetLightCount() {
            return lights->GetSize();
        }

    private:
        std::unique_ptr<pipeline::wgpu::TypedGPUBuffer<rendering::LightInfo>> lights;
    };
}
