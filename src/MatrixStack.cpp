#include "MatrixStack.h"
#include <bit>

glengine::MatrixStack2D::MatrixStack2D()
{
	stack.push_back(mat3::identity());
}

void glengine::MatrixStack2D::Push(mat3 matrix)
{
	mat3 result = stack.back() * matrix;
	stack.push_back(result);
}

void glengine::MatrixStack2D::Pop()
{
	if (stack.size() > 1) {
		stack.pop_back();
	}
}

float3 glengine::MatrixStack2D::operator*(float3 rhs)
{
	return std::bit_cast<float3>(stack.back() * std::bit_cast<vecn<float, 3>>(rhs));
}

float2 glengine::MatrixStack2D::operator*(float2 rhs)
{
	// widen to 3-wide vector
	vecn<float, 3> tmp;
	tmp[0] = rhs.x;
	tmp[1] = rhs.y;
	tmp[2] = 1.0;
	
	tmp = stack.back() * tmp;

	// narrow back
	return float2(tmp.data[0], tmp.data[1]);
}
