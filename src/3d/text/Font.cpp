//
// Created by kyle on 2026-05-31.
//

#include "Font.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

#include "Colors.h"
#include "TextRenderingResources.h"
#include "Vectors.h"
#include "pipeline/wgpu/WGPURenderer.h"

using namespace glengine::world::font;

struct SlugVertex {
    float4 pos;
    float4 color;
    uint2 glyphData;
};

class SlugGlyph {
public:
    std::vector<Curve> curves;
    float2 boundMin;
    float2 boundMax;
    float2 cursor;
};

Font::Font(pipeline::wgpu::WGPURenderer* renderer) {
    this->renderer = renderer;
    this->curves = renderer->CreateBuffer<Curve>("Font curve data", WGPUBufferUsage_Storage, 1024);
    this->curveIndices = renderer->CreateBuffer<uint32_t>("Font curve indices", WGPUBufferUsage_Storage, 1024);
    this->glyphInfos = renderer->CreateBuffer<GlyphData>("Font glyph data", WGPUBufferUsage_Storage, 64);

    auto fontBlob = hb_blob_create(embed_FiraCode_ttf, embed_FiraCode_ttf_length, HB_MEMORY_MODE_READONLY, nullptr,
                                   nullptr);
    auto face = hb_face_create(fontBlob, 0);
    font = hb_font_create(face);

    int2 iScale;
    hb_font_get_scale(font, &iScale.x, &iScale.y);
    this->scale = float2(1.0f / iScale.x, 1.0f / iScale.y);

    ttInitFont(&fontData, embed_FiraCode_ttf, embed_FiraCode_ttf_length);

    auto shaders = renderer->CompileShader(embed_slug_wgsl);

    WGPUBindGroupLayoutEntry *entries = new WGPUBindGroupLayoutEntry[3] {
        {
            .nextInChain = nullptr,
            .binding = 0,
            .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
            .bindingArraySize = 0,
            .buffer = {
                .nextInChain = nullptr,
                .type = WGPUBufferBindingType_ReadOnlyStorage,
                .hasDynamicOffset = false,
                .minBindingSize = 0
            },
            .sampler = {},
            .texture = {},
            .storageTexture = {}
        },
        {
            .nextInChain = nullptr,
            .binding = 1,
            .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
            .bindingArraySize = 0,
            .buffer = {
                .nextInChain = nullptr,
                .type = WGPUBufferBindingType_ReadOnlyStorage,
                .hasDynamicOffset = false,
                .minBindingSize = 0
            },
            .sampler = {},
            .texture = {},
            .storageTexture = {}
        },
        {
            .nextInChain = nullptr,
            .binding = 2,
            .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
            .bindingArraySize = 0,
            .buffer = {
                .nextInChain = nullptr,
                .type = WGPUBufferBindingType_ReadOnlyStorage,
                .hasDynamicOffset = false,
                .minBindingSize = 0
            },
            .sampler = {},
            .texture = {},
            .storageTexture = {}
        }
    };

    WGPUBindGroupLayoutDescriptor slugBindGroup = {
        .nextInChain = nullptr,
        .label = {},
        .entryCount = 3,
        .entries = &entries[0]
    };

    auto vtxLayouts = new WGPUVertexAttribute[3] {
        {
            .nextInChain = nullptr,
            .format = WGPUVertexFormat_Float32x4,
            .offset = offsetof(SlugVertex, pos),
            .shaderLocation = 0
        },
        {
            .nextInChain = nullptr,
            .format = WGPUVertexFormat_Float32x4,
            .offset = offsetof(SlugVertex, color),
            .shaderLocation = 2
        },
        {
        .nextInChain = nullptr,
        .format = WGPUVertexFormat_Uint32x2,
        .offset = offsetof(SlugVertex, glyphData),
        .shaderLocation = 1
        }
    };

    WGPUVertexBufferLayout layout = {
        .nextInChain = nullptr,
        .stepMode = WGPUVertexStepMode_Vertex,
        .arrayStride = sizeof(SlugVertex),
        .attributeCount = 3,
        .attributes = vtxLayouts
    };

    pipeline = renderer->BuildRenderPipeline("SLUG", shaders, &layout, std::span(&slugBindGroup, 1), sizeof(mat4));

    delete[] vtxLayouts;
    delete[] entries;

    WGPUBindGroupEntry entry = WGPU_BIND_GROUP_ENTRY_INIT;
    entry.buffer = *curves;
    pipeline->SetBinding(1, entry);

    entry.buffer = *curveIndices;
    entry.binding = 1;
    pipeline->SetBinding(1, entry);

    entry.buffer = *glyphInfos;
    entry.binding = 2;
    pipeline->SetBinding(1, entry);

    pipeline->CommitBindings();
}

std::shared_ptr<glengine::pipeline::wgpu::GPUMesh> Font::PrepareText(const char *text) {

    auto buf = hb_buffer_create();
    hb_buffer_add_utf8(buf, text, strlen(text), 0, -1);
    hb_buffer_guess_segment_properties(buf);
    hb_shape(font, buf, nullptr, 0);

    unsigned int glyphCount = 0;
    auto glyphInfos = hb_buffer_get_glyph_infos(buf, &glyphCount);

    for (int i = 0; i < glyphCount; i++) {
        printf("Prepping glyph %d\n", glyphInfos[i].codepoint);
        addGlyphToAtlas(glyphInfos[i].codepoint);
    }

    const auto dataSize = this->curveIndices->GetSize() * sizeof(uint32_t) + this->curves->GetSize() * sizeof(Curve) + this->glyphInfos->GetSize() * sizeof(GlyphData) ;
    auto session = renderer->GetTransferManager()->CreateSession("font data upload", dataSize);

    this->curves->Commit(session, [this](WGPUBuffer newBuf) {
        WGPUBindGroupEntry entry = WGPU_BIND_GROUP_ENTRY_INIT;
        entry.buffer = newBuf;
        pipeline->SetBinding(1, entry);
    });

    this->curveIndices->Commit(session, [this](WGPUBuffer newBuf) {
        WGPUBindGroupEntry entry = WGPU_BIND_GROUP_ENTRY_INIT;
        entry.buffer = newBuf;
        entry.binding = 1;
        pipeline->SetBinding(1, entry);
    });

    this->glyphInfos->Commit(session, [this](WGPUBuffer newBuf) {
        WGPUBindGroupEntry entry = WGPU_BIND_GROUP_ENTRY_INIT;
        entry.buffer = newBuf;
        entry.binding = 2;
        pipeline->SetBinding(1, entry);
    });

    session->Commit();
    pipeline->CommitBindings();

    std::vector<SlugVertex> vertices;
    vertices.reserve(glyphCount * 4);
    std::vector<uint32_t> indices;
    indices.reserve(glyphCount * 6);

    unsigned int glyphPositionCount;
    auto positions = hb_buffer_get_glyph_positions(buf, &glyphPositionCount);

    assert(glyphPositionCount == glyphCount);

    float2 offset;

    for (int i = 0; i < glyphPositionCount; i++) {
        printf("layout %c\n", text[i]);
        auto glyphIndex = glyphIndexMap[glyphInfos[i].codepoint];
        auto bounds = this->glyphInfos->operator[](glyphIndex).bounds;
        float2 min = bounds.xy;
        float2 max = bounds.zw;
        float2 minS = min * scale;
        float2 maxS = max * scale;
        int2 minI(min);
        int2 maxI(max);

        uint32_t vertexOffset = vertices.size();
        vertices.emplace_back(
            float4(offset.x + minS.x, offset.y + minS.y, 0, 1),
            Colors::WHITE,
            uint2((minI.x << 16) | (minI.y & 0xFFFF) , glyphIndex)
            );
        vertices.emplace_back(
            float4(offset.x + minS.x, offset.y + maxS.y, 0, 1),
            Colors::WHITE,
            uint2((minI.x << 16) | (maxI.y & 0xFFFF) , glyphIndex)
            );
        vertices.emplace_back(
            float4(offset.x + maxS.x, offset.y + maxS.y, 0, 1),
            Colors::WHITE,
            uint2((maxI.x << 16) | (maxI.y & 0xFFFF) , glyphIndex)
            );
        vertices.emplace_back(
            float4(offset.x + maxS.x, offset.y + minS.y, 0, 1),
            Colors::WHITE,
            uint2((maxI.x << 16) | (minI.y & 0xFFFF) , glyphIndex)
            );
        indices.insert(indices.end(), {
            vertexOffset + 0, vertexOffset + 1, vertexOffset + 2,
            vertexOffset + 0, vertexOffset + 2, vertexOffset + 3
        });


        offset.x += positions[i].x_advance * scale.x;
    }

    return renderer->UploadIndexedMesh(vertices, indices);
}

int moveTo(void* ptr, int x, int y) {
    const auto glyph = static_cast<SlugGlyph*>(ptr);
    glyph->cursor.x = x;
    glyph->cursor.y = y;

    return 1;
}

constexpr int BAND_COUNT = 8;
constexpr float LINE_EPSILON = 0.1;

int lineTo(void* ptr, int x, int y) {
    const auto glyph = static_cast<SlugGlyph*>(ptr);

    const auto start = glyph->cursor;
    const auto end = float2(x, y);
    auto controlPoint = (start + end) / 2;
    controlPoint += float2(LINE_EPSILON, LINE_EPSILON) * 0.1;

    glyph->curves.emplace_back(start, controlPoint, end);
    glyph->boundMin = min(glyph->boundMin, start);
    glyph->boundMin = min(glyph->boundMin, controlPoint);
    glyph->boundMin = min(glyph->boundMin, end);

    glyph->boundMax = max(glyph->boundMax, start);
    glyph->boundMax = max(glyph->boundMax, controlPoint);
    glyph->boundMax = max(glyph->boundMax, end);

    glyph->cursor = end;
    return 1;
}

int curveTo(void* ptr, int cx, int cy, int x, int y) {
    const auto glyph = static_cast<SlugGlyph*>(ptr);

    const auto start = glyph->cursor;
    const auto end = float2(x, y);
    const auto controlPoint = float2(cx, cy);

    glyph->curves.emplace_back(start, controlPoint, end);
    glyph->boundMin = min(glyph->boundMin, start);
    glyph->boundMin = min(glyph->boundMin, controlPoint);
    glyph->boundMin = min(glyph->boundMin, end);

    glyph->boundMax = max(glyph->boundMax, start);
    glyph->boundMax = max(glyph->boundMax, controlPoint);
    glyph->boundMax = max(glyph->boundMax, end);
    glyph->cursor = end;
    return 1;
}

struct Span {
    int start;
    int length;
};

void Font::addGlyphToAtlas(unsigned int id) {
    if (glyphIndexMap.contains(id)) return;

    auto pointCount = ttGetGlyphOutline(&fontData, id, nullptr, 0);
    auto points = new TTPoint[pointCount];
    ttGetGlyphOutline(&fontData, id, points, pointCount);

    auto glyph = new SlugGlyph();

    TTDecomposeFuncs funcs = {
        .moveTo = moveTo,
        .lineTo = lineTo,
        .curveTo = curveTo,
    };
    ttDecomposeOutline(points, pointCount, &funcs, glyph);

    GlyphData data;
    data.bounds = float4(glyph->boundMin, glyph->boundMax);
    int curveStart = this->curves->GetSize();
    for (const auto& curve : glyph->curves) {
        curves->Push(curve);
    }

    // build bands
    int bandCountH[BAND_COUNT];
    int bandCountV[BAND_COUNT];

    for (int i = 0; i < BAND_COUNT; i++) {
        bandCountH[i] = 0;
        bandCountV[i] = 0;
    }

    auto indexStart = this->curveIndices->GetSize();
    int totalIndices = 0;

    auto dimensions = glyph->boundMax - glyph->boundMin;
    for (const auto& curve : glyph->curves) {

        auto aabb = curve.BoundingBox();
        float2 cMin = aabb.xy;
        float2 cMax = aabb.zw;
        int lowerBand = std::clamp(static_cast<int>(((cMin.y - glyph->boundMin.y) / dimensions.y) * BAND_COUNT), 0, BAND_COUNT - 1);
        int upperBand = std::clamp(static_cast<int>(((cMax.y - glyph->boundMin.y) / dimensions.y) * BAND_COUNT), 0, BAND_COUNT - 1);
        for (int band = lowerBand; band <= upperBand; band++) {
            bandCountH[band]++;
            totalIndices++;
        }

        int leftBand = std::clamp(static_cast<int>(((cMin.x - glyph->boundMin.x) / dimensions.x) * BAND_COUNT), 0, BAND_COUNT - 1);
        int rightBand = std::clamp(static_cast<int>(((cMax.x - glyph->boundMin.x) / dimensions.x) * BAND_COUNT), 0, BAND_COUNT - 1);
        for (int band = leftBand; band <= rightBand; band++) {
            bandCountV[band]++;
            totalIndices++;
        }
    }

    // setup band spans
    this->curveIndices->ReserveExtra(totalIndices);
    for (int i = 0; i < totalIndices; i++) {
        this->curveIndices->Push(0);
    }
    Span horizontalBands[BAND_COUNT];
    Span verticalBands[BAND_COUNT];
    horizontalBands[0] = {
        .start = static_cast<int>(indexStart),
        .length = bandCountH[0],
    };
    bandCountH[0] = 0;
    for (int i = 1; i < BAND_COUNT; i++) {
        horizontalBands[i].start = static_cast<int>(horizontalBands[i - 1].start + horizontalBands[i - 1].length);
        horizontalBands[i].length = bandCountH[i];
        bandCountH[i] = 0;
    }

    verticalBands[0] = {
        .start = horizontalBands[BAND_COUNT - 1].start + horizontalBands[BAND_COUNT - 1].length,
        .length = bandCountV[0],
    };
    bandCountV[0] = 0;
    for (int i = 1; i < BAND_COUNT; i++) {
        verticalBands[i].start = static_cast<int>(verticalBands[i - 1].start + verticalBands[i - 1].length);
        verticalBands[i].length = bandCountV[i];
        bandCountV[i] = 0;
    }

    // build bands
    for (int curveIndex = curveStart; curveIndex < curveStart + glyph->curves.size(); curveIndex++) {
        auto& curve = this->curves->operator[](curveIndex);
        auto aabb = curve.BoundingBox();
        float2 cMin = aabb.xy;
        float2 cMax = aabb.zw;
        int lowerBand = std::clamp(static_cast<int>(((cMin.y - glyph->boundMin.y) / dimensions.y) * BAND_COUNT), 0, BAND_COUNT - 1);
        int upperBand = std::clamp(static_cast<int>(((cMax.y - glyph->boundMin.y) / dimensions.y) * BAND_COUNT), 0, BAND_COUNT - 1);
        for (int band = lowerBand; band <= upperBand; band++) {
            curveIndices->operator[](horizontalBands[band].start + bandCountH[band]) = curveIndex;
            bandCountH[band]++;
        }

        int leftBand = std::clamp(static_cast<int>(((cMin.x - glyph->boundMin.x) / dimensions.x) * BAND_COUNT), 0, BAND_COUNT - 1);
        int rightBand = std::clamp(static_cast<int>(((cMax.x - glyph->boundMin.x) / dimensions.x) * BAND_COUNT), 0, BAND_COUNT - 1);
        for (int band = leftBand; band <= rightBand; band++) {
            curveIndices->operator[](verticalBands[band].start + bandCountV[band]) = curveIndex;
            bandCountV[band]++;
        }
    }

    auto& indicesVec = curveIndices->GetStorage();
    // embed info into the glyph data
    for (int i = 0; i < BAND_COUNT; i++) {
        data.horizontalBands[i] = (horizontalBands[i].start & 0xFFFFFF) | ((horizontalBands[i].length << 24) & 0xFF000000);
        data.verticalBands[i] = (verticalBands[i].start & 0xFFFFFF) | (verticalBands[i].length << 24);

        // sort horizontal band i
        std::sort(indicesVec.begin() + horizontalBands[i].start, indicesVec.begin() + horizontalBands[i].length + horizontalBands[i].start, [this](const uint32_t& a, const uint32_t& b) {
            auto a_maxX = curves->operator[](a).BoundingBox().z;
            auto b_maxX = curves->operator[](b).BoundingBox().z;
            return a_maxX > b_maxX;
        });

        // sort vertical band i
        std::sort(indicesVec.begin() + verticalBands[i].start, indicesVec.begin() + verticalBands[i].length + verticalBands[i].start, [this](const uint32_t& a, const uint32_t& b) {
            auto a_maxY = curves->operator[](a).BoundingBox().w;
            auto b_maxY = curves->operator[](b).BoundingBox().w;
            return a_maxY > b_maxY;
        });
    }

    this->glyphInfos->Push(data);
    delete glyph;

    glyphIndexMap.insert({id, this->glyphInfos->GetSize() - 1});
}
