#pragma once
#include "Matrix.h"
#include "Vectors.h"
#include <vector>

namespace glengine {
	/// <summary>
	/// Stack of matricies, for nested 2D transforms.
	/// </summary>
	class MatrixStack2D {
	public:
		MatrixStack2D();

		/// <summary>
		/// Applies a new transform. The previous top of the stack will be multiplied by the given matrix,
		/// then the result will be pushed to the top of the stack.
		/// </summary>
		void Push(mat3 matrix);

		/// <summary>
		/// Pops a transform from the stack.
		/// </summary>
		void Pop();

		/// <summary>
		/// Conveinence overload for matrix-vector multiplication.
		/// </summary>
		float3 operator*(float3 rhs);

		/// <summary>
		/// Conveinence overload for transforming 2d points. Internally widens to a vec3 with z = 1.0.
		/// 
		/// Less efficent than the float3 version.
		/// </summary>
		float2 operator*(float2 rhs);

		/// <summary>
		/// Conveinence method for drawing a polygon, transformed by the top transform on the stack.
		/// </summary>
		void DrawPolygon(std::vector<float3> verticies);

		/// <summary>
		/// Draws a rectangle using point a as the bottom left and point b as the top right.
		/// </summary>
		void DrawRect(float2 a, float2 b);

	private:
		std::vector<mat3> stack; 
	};
}
