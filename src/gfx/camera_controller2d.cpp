#include "camera_controller2d.h"

#include<algorithm>
#include<cmath>
#include<GLFW/glfw3.h>


namespace argon {
	CameraController2D::CameraController2D(Window& win, Camera2D& cam)
		: m_win(win), m_cam(cam) {}

	void CameraController2D::update(float dt) {

		float dx = 0.0f, dy = 0.0f;
		if (m_win.keyDown(GLFW_KEY_A) || m_win.keyDown(GLFW_KEY_LEFT)) dx -= 1.0f;
		if (m_win.keyDown(GLFW_KEY_D) || m_win.keyDown(GLFW_KEY_RIGHT)) dx += 1.0f;
		if (m_win.keyDown(GLFW_KEY_W) || m_win.keyDown(GLFW_KEY_UP)) dy += 1.0f;
		if (m_win.keyDown(GLFW_KEY_S) || m_win.keyDown(GLFW_KEY_DOWN)) dy -= 1.0f;

		if (dx != 0.0f || dy != 0.0f) {
			float len = std::sqrt(dx * dx + dy * dy);
			dx /= len;
			dy /= len;
			m_cam.x += dx * moveSpeed * dt / m_cam.zoom;
			m_cam.y += dy * moveSpeed * dt / m_cam.zoom;
		}

		if (m_win.keyDown(GLFW_KEY_Q)) m_cam.rotation += rotSpeed * dt;
		if (m_win.keyDown(GLFW_KEY_E)) m_cam.rotation -= rotSpeed * dt;

		double sy = m_win.scrollDeltaY();
		if (sy != 0.0) {
			float factor = std::exp((float)sy * zoomSpeed);
			m_cam.zoom *= factor;
			m_cam.zoom = std::clamp(m_cam.zoom, minZoom, maxZoom);
		}

		if (m_win.mouseDown(GLFW_MOUSE_BUTTON_RIGHT)) {
			double mdx = 0.0, mdy = 0.0;
			m_win.mouseDelta(mdx, mdy);

			int fbW = m_win.framebufferWidth();
			int fbH = m_win.framebufferHeight();
			float aspect = (fbH != 0) ? (float)fbW / (float)fbH : 1.0f;

			float halfH = m_cam.size;
			float halfW = m_cam.size * aspect;

			float unitsPerPixelX = (2.0f * halfW) / std::max(1, fbW) / m_cam.zoom;
			float unitsPerPixelY = (2.0f * halfH) / std::max(1, fbH) / m_cam.zoom;

			m_cam.x -= (float)mdx * unitsPerPixelX;
			m_cam.y += (float)mdy * unitsPerPixelY;

		}

	}

	
		
}
