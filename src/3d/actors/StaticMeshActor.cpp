//
// Created by Kyle Smith on 2026-06-09.
//

#include "3d/actors/StaticMeshActor.h"

#include "Engine.h"
#include "3d/mesh/StaticMeshComponent.h"


namespace glengine::world::actors {
    StaticMeshActor::StaticMeshActor(std::string_view resourceName) {
        auto mesh = GetEngine()->GetResourceManager()->GetResource<mesh::StaticMesh>(resourceName);
        auto model = CreateComponent<mesh::StaticMeshComponent>();
        model->SetMesh(mesh);
        Material = model->material;
    }

    StaticMeshActor::StaticMeshActor(std::shared_ptr<mesh::StaticMesh> mesh) {
        auto model = CreateComponent<mesh::StaticMeshComponent>();
        model->SetMesh(std::move(mesh));
        Material = model->material;
    }

    void StaticMeshActor::Update(double deltaTime) {
        Material.Commit();
    }
}
