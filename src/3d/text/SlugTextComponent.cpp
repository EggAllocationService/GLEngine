//
// Created by Kyle Smith on 2026-05-31.
//

#include "SlugTextComponent.h"

#include "Engine.h"


namespace glengine::world {
    namespace text {
    } // text
    font::SlugTextComponent::SlugTextComponent() {
        font = new Font(GetEngine()->GetRenderer());
        mesh = font->PrepareText("Hello!");
    }

    font::SlugTextComponent::~SlugTextComponent() {
        delete font;
    }

    void font::SlugTextComponent::Render(const pipeline::wgpu::RenderBundle &bundle, MatrixStack &stack) {
        mat4 transform = stack;
        font->GetPipeline()->DrawMesh(bundle, *mesh, &transform);
    }
}