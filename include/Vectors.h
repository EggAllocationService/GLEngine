//
// Created by Kyle Smith on 2025-09-26.
//
#pragma once

/// Helper type to allow for glsl-like vector swizzling
/// @tparam vector the type of the swizzled vector, possibly narrowed from the source
/// @tparam primitive is the primitive type being stored in the vector, e.g. `float`
/// @tparam len is the number of lanes in the _source_ vector, *not* the swizzled vector
/// @tparam indices are the mappings from the source lanes to each lane of the swizzled vector.
///
/// Example usage for a 3-wide int vector:
/// @code
/// union {
///     int data[3]; // the actual storage
///
///     // setup parameters:
///     // `vec2<int>` indicates this swizzled representation can be converted to a vec2<int>
///     // `int` indicates the primitive type stored is an int, allowing for broadcast assignments (foo.xy = 3)
///     // `3` sets the source vector width to 3 (so it fits in the union correctly)
///     // `0, 1` indicates there are two elements in the swizzled vector, from source lane 0 and 1 respectively.
///     vec_swizzle<vec2<int>, int, 3, 0, 1> xy;
///
///     // we can have a reversed version of the above by reversing the order of `indices`:
///     vec_swizzle<vec2<int>, int, 3, 1, 0> yx;
/// }
/// @endcode
template<typename vector, typename primitive, int len, int... indices>
struct vec_swizzle {
    float data[len];

    vector operator=(vector rhs) {
        return vector((data[indices] = rhs[indices])...);
    }

    vector operator=(primitive rhs) {
        return vector((data[indices] = rhs)...);
    }

    operator vector() {
        return vector(data[indices]...);
    }
};

/// Constant-length vector, templated.
///
/// Useful for using in other templated code, for e.g. templated matrix multiplication
/// @tparam T primitive type of vector
/// @tparam LEN vector width
template<typename T, int LEN>
struct vecn {
    vecn() {
    }

    static vecn zero() {
        vecn ret;
        for (int i = 0; i < LEN; i++) ret.data[i] = 0;
        return ret;
    }

    T data[LEN];

    void set(int index, T value) {
        data[index] = value;
    }

    T &operator[](int index) {
        return data[index];
    }

    vecn operator*(vecn<T, LEN> &rhs) {
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

    void operator+=(vecn<T, LEN> rhs) {
        for (int i = 0; i < LEN; i++) {
            data[i] += rhs.data[i];
        }
    }
};

/// Constant-width vector, 2 lanes
/// @tparam T primitive type
template<typename T>
struct vec2 {
    vec2() {
        data[0] = 0;
        data[1] = 0;
    }

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

        vec_swizzle<vec2, T, 2, 0, 1> xy, rg;
        vec_swizzle<vec2, T, 2, 1, 0> yx, gr;
        vec_swizzle<vec2, T, 2, 0, 0> xx, rr;
        vec_swizzle<vec2, T, 2, 1, 1> yy, gg;
    };

    operator T *() {
        return &data[0];
    }

    bool operator==(const vec2<T> &rhs) {
        return data[0] == rhs.data[0] && data[1] == rhs.data[1];
    }
};

/// Constant-width vector, 3 lanes
/// @tparam T primitive type
template<typename T>
struct vec3 {
    vec3() {
    }

    vec3(T x, T y, T z) {
        data[0] = x;
        data[1] = y;
        data[2] = z;
    }

    vec3(vec2<T> xy, T z) {
        data[0] = xy.x;
        data[1] = xy.y;
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

        vec_swizzle<vec3, T, 3, 0, 1, 2> xyz, rgb;
        vec_swizzle<vec3, T, 3, 2, 1, 0> zyx, bgr;
        vec_swizzle<vec2<T>, T, 3, 0, 1> xy, rg;
        vec_swizzle<vec2<T>, T, 3, 1, 0> yx, gr;
    };

    operator T *() {
        return &data[0];
    }

    bool operator==(const vec3<T> &rhs) {
        for (int i = 0; i < 3; i++) {
            if (data[i] != rhs.data[i]) return false;
        }
        return true;
    }
};

/// Constant-width vector, 4 lanes
/// @tparam T primitive type
template<typename T>
struct vec4 {
    vec4() {
    }

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

        vec_swizzle<vec4, T, 4, 0, 1, 2, 3> xyzw, rgba;
        vec_swizzle<vec4, T, 4, 0, 1, 2, 3> wzyx, abgr;
        vec_swizzle<vec3<T>, T, 4, 0, 1, 2> xyz, rgb;
        vec_swizzle<vec3<T>, T, 4, 2, 1, 0> zyx, bgr;
        vec_swizzle<vec2<T>, T, 4, 0, 1> xy, rg;
        vec_swizzle<vec2<T>, T, 4, 1, 0> yx, gr;
    };

    operator const T *() const {
        return &x;
    }

    vec4 operator *(const vec4<T> &other) {
        return vec4(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    vec4 operator +(const vec4<T> &other) {
        return vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    void operator +=(const vec4<T> other) {
        for (int i = 0; i < 4; i++) {
            this->data[i] += other.data[i];
        }
    }

    bool operator==(const vec4<T> &other) {
        for (int i = 0; i < 4; i++) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }
};

// basic vector types for convenience
typedef vec4<float> float4;
typedef vec4<int> int4;

typedef vec3<float> float3;
typedef vec3<int> int3;

typedef vec2<float> float2;
typedef vec2<int> int2;
