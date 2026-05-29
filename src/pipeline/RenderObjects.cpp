#include "pipeline/RenderObjects.h"

#include "engine_GLUT.h"
#include <algorithm>

using namespace glengine::rendering;

glengine::pipeline::wgpu::WGPURenderer* glengine::pipeline::RenderObject::CURRENT_RENDERER = nullptr;

RenderObjects::RenderObjects(glengine::pipeline::wgpu::WGPURenderer* renderer) {
    this->renderer = renderer;
};