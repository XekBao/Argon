#pragma once
#include "math/mat4.h"

namespace argon {
	
	class Camera2D {
	public:
		float x = 0.0f;
		float y = 0.0f;
		float rotation = 0.0f;
		float zoom = 1.0f;

		float size = 1.0f;

		Mat4 projection(float aspect) const {
			float halfH = size / zoom;
			float halfW = size * aspect /zoom;
			return Mat4::ortho(-halfW, halfW, -halfH, halfH);
		}

		Mat4 view() const {
			Mat4 Tinv = Mat4::translate(-x, -y, 0.0f);
			Mat4 Rinv = Mat4::rotateZ(-rotation);

			return mul(Rinv, Tinv);
		}

		Mat4 projView(float aspect) const {
			return mul(projection(aspect), view());
		}

	private:
	};

}