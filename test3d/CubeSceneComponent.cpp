//
// Created by Kyle Smith on 2025-10-18.
//

#include "engine_GLUT.h"
#include "CubeSceneComponent.h"

#include "Colors.h"

void CubeSceneComponent::Render() {
    //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, reinterpret_cast<float*>(&Color));
    glutSolidSphere(Scale, 30, 30);
}
