//
// Created by Kyle Smith on 2026-06-09.
//

#pragma once
#include "3d/Actor.h"
#include "3d/mesh/StaticMesh.h"

namespace glengine::world::actors {
    class StaticMeshActor: public Actor {
    public:
        StaticMeshActor(std::string_view resourceName);
        StaticMeshActor(std::shared_ptr<mesh::StaticMesh> mesh);

        void Update(double deltaTime) override;

        pipeline::wgpu::GPUPointer<mesh::Material> Material;
    };
}
