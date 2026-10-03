//
// Created by Kyle Smith on 2026-10-01.
//

#ifndef GLENGINE_OPTIMIZEDMESHCOMPONENT_H
#define GLENGINE_OPTIMIZEDMESHCOMPONENT_H
#include "OptimizedMesh.h"
#include "3d/ActorPrimitiveComponent.h"
#include "3d/objects/OptimizedDrawTracker.h"

namespace glengine::world::mesh {
    class OptimizedMeshComponent : public ActorPrimitiveComponent {
    public:
        GLENGINE_EXPORT OptimizedMeshComponent();
        GLENGINE_EXPORT OptimizedMeshComponent(std::string_view name);

        GLENGINE_EXPORT void SetMesh(std::shared_ptr<OptimizedMesh>& mesh) {
            mesh_ = mesh;
        }

        GLENGINE_EXPORT void Update(double deltaTime) override;

    private:
        std::shared_ptr<OptimizedMesh> mesh_;
        std::shared_ptr<objects::OptimizedDrawTracker> tracker_;
    };
}

#endif //GLENGINE_OPTIMIZEDMESHCOMPONENT_H
