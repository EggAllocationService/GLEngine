#include "MatrixStack.h"
#include "engine_GLUT.h"
#include <bit>

glengine::MatrixStack2D::MatrixStack2D() {
    stack.push_back(mat3::identity());
}

void glengine::MatrixStack2D::Push(mat3 matrix) {
    mat3 result = stack.back() * matrix;
    stack.push_back(result);
}

void glengine::MatrixStack2D::Pop() {
    if (stack.size() > 1) {
        stack.pop_back();
    }
}

float3 glengine::MatrixStack2D::operator*(float3 rhs) {
    // need to convert between float3 and vecn<float, 3> as matrix multiplication is only defined
    // with vecn instantiations, so the compiler can generate the most optimal machine code for each size

    // in this case std::bit_cast will be a no-op as float3 and vecn<float, 3> are the same size
    // and are both value types
    return std::bit_cast<float3>(stack.back() * std::bit_cast<vecn<float, 3>>(rhs));
}

float2 glengine::MatrixStack2D::operator*(float2 rhs) {
    // widen to 3-wide vector
    vecn < float, 3 > tmp;
    tmp[0] = rhs.x;
    tmp[1] = rhs.y;
    tmp[2] = 1.0;

    tmp = stack.back() * tmp;

    // narrow back
    return float2(tmp.data[0], tmp.data[1]);
}

void glengine::MatrixStack2D::DrawPolygon(std::vector<float3>& verticies) {
    glBegin(GL_POLYGON);
    for (float3 &vertex: verticies) {
        glVertex3fv(this->operator*(vertex));
    }
    glEnd();
}

void glengine::MatrixStack2D::DrawRect(float2 a, float2 b) {
    float3 verticies[4] = {
        float3(a, 1.0),
        float3(a.x, b.y, 1.0),
        float3(b, 1.0),
        float3(b.x, a.y, 1.0)
    };

    glBegin(GL_POLYGON);
    for (auto vertex : verticies) {
        glVertex3fv(this->operator*(vertex));
    }
    glEnd();
}

void glengine::MatrixStack2D::PrintText(float2 position, const char* text)
{
    int pos = 0;
    
    for (const char* current = text; *current != 0; current++) {
        
        glRasterPos2fv(this->operator*(position + float2(8.0 * pos, 0)));

        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *current);

        pos++;
    }
}
