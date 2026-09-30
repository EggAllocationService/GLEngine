//
// Created by Kyle Smith on 2026-09-30.
//

#ifndef GLENGINE_MESHDEMOCOMPONENT_H
#define GLENGINE_MESHDEMOCOMPONENT_H
#include "3d/ActorSceneComponent.h"


class MeshDemoComponent : public glengine::world::ActorSceneComponent {

public:
    MeshDemoComponent();
    void Render(const glengine::pipeline::wgpu::RenderBundle &, glengine::MatrixStack &stack) override;

    static void RegisterPipeline(glengine::Engine* engine);
private:
    std::shared_ptr<glengine::pipeline::wgpu::MeshPipeline> pipeline_;
};


#endif //GLENGINE_MESHDEMOCOMPONENT_H
