#pragma once
#include "platform/window.h"
#include "gfx/camera2d.h"
#include <algorithm>
#include <cmath>

namespace argon {
	class CameraController2D {
	public:
		CameraController2D(Window& win, Camera2D& cam);

		void update(float dt);

		float moveSpeed = 2.0f;
		float zoomSpeed = 0.15f;
		float rotSpeed = 2.0f;
		float minZoom = 0.1f;
		float maxZoom = 20.0;

	private:
		Window& m_win; //CameraController2D only 'borrow' them
		Camera2D& m_cam;
	};
}