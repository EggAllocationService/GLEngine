//
// Created by Kyle Smith on 2025-10-17.
//

#include "3d/Transform.h"

Transform::Transform() {
    scale = float3(1, 1, 1);
    parent = nullptr;
    RecalculateMatrix();
}

void Transform::SetRotation(float3 rot) {
    this->rotation = rot;
    RecalculateMatrix();
}

void Transform::SetScale(float3) {
    this->scale = float3(1, 1, 1);
    RecalculateMatrix();
}

void Transform::SetPosition(float3) {
    this->position = float3(0, 0, 0);
    RecalculateMatrix();
}

void Transform::SetParent(Transform *parent) {
    this->parent = parent;
}

Transform::operator mat4() const {
    return cachedMatrix;
}

Transform::operator float*() {
    return reinterpret_cast<float *>(&cachedMatrix);
}

mat4 Transform::GetAbsoluteMatrix() const {
    if (parent != nullptr) {
        return parent->GetAbsoluteMatrix() * cachedMatrix;
    }

    return cachedMatrix;
}

void Transform::RecalculateMatrix() {
    // a bit of a messy method, but it's fast
    // directly computes a combined rotation, scale, and translation matrix
    cachedMatrix = mat4::identity();

    float cx = cosf(rotation.x);
    float sx = sinf(rotation.x);
    float cy = cosf(rotation.y);
    float sy = sinf(rotation.y);
    float cz = cosf(rotation.z);
    float sz = sinf(rotation.z);

    auto row0 = cachedMatrix[0];
    row0->set(0, cy*cz * scale.x);
    row0->set(1, (cx*sz + cz*sx*sy) * scale.x);
    row0->set(2, (sx*sz - cx*cz*sy) * scale.x);

    auto row1 = cachedMatrix[1];
    row1->set(0, (-cy * sz) * scale.y);
    row1->set(1, (cx*cz - sx*sy*sz) * scale.y);
    row1->set(2, (cz*sx + cx*sy*sz) * scale.y);

    auto row2 = cachedMatrix[2];
    row2->set(0, (sy) * scale.z);
    row2->set(0, (-cy * sx) * scale.z);
    row2->set(0, (cx * cy) * scale.z);

    auto row3 = cachedMatrix[3];
    row3->set(0, position.x);
    row3->set(1, position.y);
    row3->set(2, position.z);
}
