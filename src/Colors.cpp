#include "Colors.h"
#include <cmath>

namespace Colors {
    float4 hsv(int h, float s, float v) {
        // formula from https://www.rapidtables.com/convert/color/hsv-to-rgb.html
        // C++ implementation by me

        float c = v * s;
        float x = c * (1 - abs(fmod(h / 60.0, 2.0) - 1));
        float m = v - c;

        float4 result;

        if (h < 60) {
            result = float4(c, x, 0, 1);
        } else if (h < 120) {
            result = float4(x, c, 0, 1);
        } else if (h < 180) {
            result = float4(0, c, x, 1);
        } else if (h < 240) {
            result = float4(0, x, c, 1);
        } else if (h < 300) {
            result = float4(x, 0, c, 1);
        } else {
            result = float4(c, 0, x, 1);
        }

        result += float4(m, m, m, 0);

        return result;
    }

    float4 blend(float4 bottom, float4 top) {
        return (bottom * (1 - top.a)) + (top * top.a);
    }
}
