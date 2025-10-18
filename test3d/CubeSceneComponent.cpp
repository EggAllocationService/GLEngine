//
// Created by Kyle Smith on 2025-10-18.
//

#include "engine_GLUT.h"
#include "CubeSceneComponent.h"

#include "Colors.h"

void CubeSceneComponent::Render() {
    glColor4fv(Colors::RED);
    glutWireCube(1.0);
}
