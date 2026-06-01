//
// Created by Kyle Smith on 2026-05-31.
//

#include "3d/text/SlugTextComponent.h"

#include "Engine.h"


namespace glengine::world {

    font::SlugTextComponent::SlugTextComponent() = default;

    void font::SlugTextComponent::Render(const pipeline::wgpu::RenderBundle &bundle, MatrixStack &stack) {
        if (font == nullptr || text.empty()) {
            return;
        }

        mat4 transform = stack;
        font->GetPipeline()->DrawMesh(bundle, *mesh, &transform);
    }

    void font::SlugTextComponent::SetFont(std::shared_ptr<Font> newFont) {
        this->font = newFont;
        if (!this->text.empty()) {
            this->mesh = this->font->PrepareText(text.c_str());
        }
    }

    void font::SlugTextComponent::SetText(std::string_view newText) {
        this->text = newText;
        if (this->font != nullptr) {
            this->mesh = this->font->PrepareText(this->text.c_str());
        }
    }
}
