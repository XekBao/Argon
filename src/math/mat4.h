#pragma once
#include <cmath>

namespace argon {

	struct Mat4 {
		float m[16];

		static Mat4 identity() {
			Mat4 r{};
			r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
			return r;
		}

		static Mat4 translate(float x, float y, float z = 0.0f) {
			Mat4 r = identity();
			r.m[12] = x;
			r.m[13] = y;
			r.m[14] = z;
			return r;
		}

		static Mat4 scale(float sx, float sy, float sz = 1.0f) {
			Mat4 r{};
			r.m[0] = sx;
			r.m[5] = sy;
			r.m[10] = sz;
			r.m[15] = 1.0f;
			return r;
		}

		static Mat4 rotateZ(float radians) {
			Mat4 r = identity();
			float c = std::cos(radians);
			float s = std::sin(radians);

			r.m[0] = c; r.m[4] = -s;
			r.m[1] = s; r.m[5] = c;
			return r;
		}

		static Mat4 ortho(float left, float right, float bottom, float top, float zNear = -1.0f, float zFar = 1.0f) {
			Mat4 r{};
			r.m[0] = 2.0f / (right - left);
			r.m[5] = 2.0f / (top - bottom);
			r.m[10] = -2.0f / (zFar - zNear);
			r.m[15] = 1.0f;

			r.m[12] = -(right + left) / (right - left);
			r.m[13] = -(top + bottom) / (top - bottom);
			r.m[14] = -(zFar + zNear) / (zFar - zNear);
			return r;
		}
	};

	inline Mat4 mul(const Mat4& A, const Mat4& B) {
		Mat4 R{};
		// R = A * B, column matrix
		for (int col = 0; col < 4; ++col) {
			for (int row = 0; row < 4; ++row) {
				R.m[col * 4 + row] =
					A.m[0 * 4 + row] * B.m[col * 4 + 0] +
					A.m[1 * 4 + row] * B.m[col * 4 + 1] +
					A.m[2 * 4 + row] * B.m[col * 4 + 2] +
					A.m[3 * 4 + row] * B.m[col * 4 + 3];
			}
		}
		return R;
	}

}