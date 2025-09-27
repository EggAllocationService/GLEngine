//
// Created by Kyle Smith on 2025-09-26.
//
#pragma once

template <typename T, int LEN>
struct vecn {
    vecn() {}

    static vecn zero() {
        vecn ret;
        for (int i = 0; i < LEN; i++) ret.data[i] = 0;
        return ret;
    }

    T data[LEN];

    T operator[](int index) {
        return data[index];
    }

    vecn operator*(vecn<T, LEN>& rhs) {
        vecn<T, LEN> result;
        for (int i = 0; i < LEN; i++) {
            result.data[i] = data[i] * rhs.data[i];
        }
        return result;
    }

    template<typename I>
    vecn operator*(I rhs) {
        vecn<T, LEN> result;

        for (int i = 0; i < LEN; i++) {
            result.data[i] = data[i] * rhs;
        }
        return result;
    }

    vecn operator+(vecn<T, LEN> rhs) {
        vecn<T, LEN> result;

        for (int i = 0; i < LEN; i++) {
            result.data[i] = data[i] + rhs.data[i];
        }
        return result;
    }
};

template <typename T>
struct vec4 {
    vec4 () {}

    vec4(T x, T y, T z, T w) {
        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;
    }
    union {
        T data[4];
        struct {
            T x, y, z, w;
        };
        struct {
            T r, g, b, a;
        };
    };

    operator const T*() const {
        return &x;
    }

    vec4 operator *(const vec4<T> &other) {
        return vec4(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    vec4 operator +(const vec4<T> &other) {
        return vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }
};

template <typename T>
struct vec3 {
    vec3() {}

    vec3(T x, T y, T z) {
        data[0] = x;
        data[1] = y;
        data[2] = z;
    }
    union {
        T data[3];
        struct {
            T x, y, z;
        };
        struct {
            T r, g, b;
        };
    };

    operator T*() const {
        return &data;
    }
};

template <typename T>
struct vec2 {
    vec2() {}

    vec2(T x, T y) {
        data[0] = x;
        data[1] = y;
    }
    union {
        T data[2];
        struct {
            T x, y;
        };
        struct {
            T r, g;
        };
    };

    operator T*() const {
        return &data;
    }
};


typedef vec4<float> float4;
typedef vec4<int> int4;

typedef vec3<float> float3;
typedef vec3<int> int3;

typedef vec2<float> float2;
typedef vec2<int> int2;