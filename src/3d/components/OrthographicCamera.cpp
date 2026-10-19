//
// Created by kyle on 2026-10-19.
//

#include "3d/components/OrthographicCamera.h"

#include "GLMath.h"


namespace glengine::world::components {
    mat4 OrthographicCamera::GetProjectionMatrix() {
        return math::orthographicMatrix(Left, Right, Bottom, Top, Near, Far);
    }
}