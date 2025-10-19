//
// Created by Kyle Smith on 2025-10-18.
//

#include "engine_GLUT.h"
#include "CubeSceneComponent.h"

#include "Colors.h"

void CubeSceneComponent::Render() {
    glColor4fv(Color);
    glutWireCube(Scale);
}
