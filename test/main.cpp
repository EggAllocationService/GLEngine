//
// Created by Kyle Smith on 2025-09-26.
//

#include "Matrix.h"
#include <iostream>
#include "Engine.h"
#include "engine_GLUT.h"
#include "RgbTriangle.h"

using namespace glengine;

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    auto inst = new Engine("Hello World!", int2(600, 600));

    inst->AddOnscreenWidget(new RgbTriangle());

    glutMainLoop();
}
