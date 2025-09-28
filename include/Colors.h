//
// Created by Kyle Smith on 2025-09-26.
//

#pragma once
#include "Vectors.h"

namespace Colors {
    static float4 BLACK = float4(0.0, 0.0, 0.0, 1.0);
    static float4 RED = float4(1.0, 0.0, 0.0, 1.0);
    static float4 GREEN = float4(0.0, 1.0, 0.0, 1.0);
    static float4 BLUE = float4(0.0, 0.0, 1.0, 1.0);
    static float4 WHITE = float4(1.0, 1.0, 1.0, 1.0);

    /// <summary>
    /// Creates an RGBA color from HSV
    /// </summary>
    /// <param name="h">Hue, in degrees. 0 < h < 360</param>
    /// <param name="s">Saturation, 0.0-1.0</param>
    /// <param name="v">Value, 0.0-1.0</param>
    /// <returns>A rgba color. Alpha will always be 1.0</returns>
    float4 hsv(int h, float s, float v);
}