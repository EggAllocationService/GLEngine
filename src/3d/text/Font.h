//
// Created by kyle on 2026-05-31.
//
#pragma once

#include <hb.h>
#include "truetype.h"
#include "pipeline/wgpu/TypedGPUBuffer.h"
#include "pipeline/wgpu/pipeline/RenderPipeline.h"
#include "pipeline/wgpu/GPUMesh.h"
#include "Vectors.h"

namespace glengine::world::font {
    struct Curve {
        float2 p0, p1, p2;

        [[nodiscard]] float4 BoundingBox() const {
            float2 m = min(min(p0, p1), p2);
            float2 M = max(max(p0, p1), p2);
            return {m, M};
        }
    };

    struct alignas(16) GlyphData {
        uint32_t verticalBands[8];
        uint32_t horizontalBands[8];
        float4 bounds;
    };


    class Font {
    public:
        Font(pipeline::wgpu::WGPURenderer *Renderer);

        std::shared_ptr<pipeline::wgpu::GPUMesh> PrepareText(const char* text);

        [[nodiscard]] std::shared_ptr<pipeline::wgpu::RenderPipeline> GetPipeline() const {
            return pipeline;
        }

    private:
        void addGlyphToAtlas(unsigned int id);
        TTFont fontData;
        std::unique_ptr<pipeline::wgpu::TypedGPUBuffer<Curve>> curves;
        std::unique_ptr<pipeline::wgpu::TypedGPUBuffer<uint32_t>> curveIndices;
        std::unique_ptr<pipeline::wgpu::TypedGPUBuffer<GlyphData>> glyphInfos;
        std::unordered_map<uint32_t, uint32_t> glyphIndexMap;
        hb_font_t *font;
        pipeline::wgpu::WGPURenderer* renderer;
        std::shared_ptr<pipeline::wgpu::RenderPipeline> pipeline;

        float2 scale;

    };
}