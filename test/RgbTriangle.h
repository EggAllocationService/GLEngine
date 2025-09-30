#pragma once

#include "Widget.h"
#include "widgets/Button.h"
using namespace glengine;

class RgbTriangle : public Widget {
public:
    RgbTriangle();

    virtual void Update(double DeltaTime) override;

    virtual void Draw(MatrixStack2D &stack) override;
    int2 size;
    float2 velocity;

private:
    float hue;

    float innerRotation;

    float rotationScalar = 1.0f;

    std::shared_ptr<widgets::Button> button;
};