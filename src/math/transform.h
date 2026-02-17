#pragma once
#include "mat4.h"

namespace argon {
	struct Transform {
		float x = 0.0f;
		float y = 0.0f;
		float rotation = 0.0f;
		float sx = 1.0f;
		float sy = 1.0f;

		Mat4 matrix() const {
			Mat4 T = Mat4::translate(x, y, 0.0f);
			Mat4 R = Mat4::rotateZ(rotation);
			Mat4 S = Mat4::scale(sx, sy, 1.0f);
			return mul(T, mul(R, S));
		}
	};
}