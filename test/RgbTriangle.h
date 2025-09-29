#pragma once

#include "Widget.h"
using namespace glengine;

class RgbTriangle : public Widget {
public:
    RgbTriangle();

    virtual void Update(double DeltaTime) override;

    virtual void Draw(MatrixStack2D &stack) override;

private:
    float hue;
    float2 size;
    float2 velocity;

    float innerRotation;
};