#pragma once
#include "3d/ActorPrimitiveComponent.h"
#include "glengine_export.h"

namespace glengine::world::components {
    /// Represents a viewpoint into the world
    class GLENGINE_EXPORT PointLightComponent : public ActorPrimitiveComponent {
    public:
        void Update(double) override;

        float4 Diffuse = float4(0.3, 0.3, 0.3, 1);
        float4 Specular = float4(1, 1, 1, 1);
        float Intensity = 1.0;
    };
}
