#pragma once

#include "Widget.h"
#include "widgets/Button.h"
using namespace glengine;

class RgbTriangle : public Widget {
public:
    RgbTriangle();

    virtual void Update(double DeltaTime) override;

    virtual void Draw(MatrixStack2D &stack) override;

    void Click(int button, int state, float2 pos) override;
    int2 size;

private:
    void bringToFront();

    float hue = 0.0;

    float innerRotation = 0.0;

    float rotationScalar = 1.0f;

    float2 clickPos;

    bool pause = false;

    std::shared_ptr<widgets::Button> button;
};