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

	private:
		std::vector<mat3> stack; 
	};
}
