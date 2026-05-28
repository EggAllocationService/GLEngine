//
// Created by Kyle Smith on 2025-10-21.
//
#pragma once
#include "glengine_export.h"
#include "StaticMesh.h"
#include "3d/ActorSceneComponent.h"
#include "3d/texture/StaticTexture2D.h"

namespace glengine::world::mesh {
    class GLENGINE_EXPORT StaticMeshComponent : public ActorSceneComponent {
    public:
        StaticMeshComponent();
        void Render(const pipeline::wgpu::RenderBundle&, MatrixStack&) override;

        void SetMesh(std::shared_ptr<StaticMesh>);
        void SetTexture(std::shared_ptr<texture::StaticTexture2D>);

        Material material;
    private:
        std::shared_ptr<StaticMesh> mesh_;
        std::shared_ptr<texture::StaticTexture2D> texture_;
        std::shared_ptr<pipeline::wgpu::RenderPipeline> pipeline_;
    };
}
