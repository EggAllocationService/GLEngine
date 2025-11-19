#pragma once

#include "glengine_export.h"
#include "PointLightComponent.h"

namespace glengine::world::components {
    /// Represents a viewpoint into the world
    class GLENGINE_EXPORT DirectionalLightComponent : public PointLightComponent {
    public:
        void Update(double) override;
    };
}
