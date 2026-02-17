#include "platform/window.h"
#include <iostream>

namespace argon {
	void Window::scrollCallback(GLFWwindow* win, double xoff, double yoff) {
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
		if (!self)return;
		self->m_scrollY += yoff;
	}

	void Window::glfwErrorCallback(int code, const char* desc) {
		std::cerr << "[GLFW Error] (" << code << ")" << desc << "\n";
	}

	void Window::framebufferSizeCallback(GLFWwindow* win, int w, int h) {
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
		if (!self)return;

		self->m_fbWidth = w;
		self->m_fbHeight = h;
	}


	Window::Window(int width, int height, const char* title) {
		glfwSetErrorCallback(glfwErrorCallback);

		if (!glfwInit()) {
			std::cerr << "GLFW init failed\n";
			return;
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (!m_window) {
			std::cerr << "Window creation failed\n";
			glfwTerminate();
			return;
		}

		glfwMakeContextCurrent(m_window);
		glfwSetWindowUserPointer(m_window, this);
		glfwGetFramebufferSize(m_window, &m_fbWidth, &m_fbHeight);
		
		glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
		glfwSetScrollCallback(m_window, scrollCallback);

		glfwGetCursorPos(m_window, &m_mouseX, &m_mouseY);
		m_mouseDX = m_mouseDY = 0.0;
		m_scrollY = 0.0;

		setVsync(true);
	}

	Window::~Window() {
		if (m_window) {
			glfwDestroyWindow(m_window);
			m_window = nullptr;
			glfwTerminate();
		}
	}

	bool Window::shouldClose() const {
		return m_window && glfwWindowShouldClose(m_window);
	}

	void Window::swapBuffers() const {
		if (m_window) glfwSwapBuffers(m_window);
	}

	void Window::pollEvents() {

		m_mouseDX = 0.0;
		m_mouseDY = 0.0;
		m_scrollY = 0.0;

		glfwPollEvents();

		double x = 0.0, y = 0.0;
		glfwGetCursorPos(m_window, &x, &y);

		m_mouseDX = x - m_mouseX;
		m_mouseDY = y - m_mouseY;

		m_mouseX = x;
		m_mouseY = y;
	}

	void Window::setVsync(bool enabled) {
		glfwSwapInterval(enabled ? 1 : 0);
	}

	bool Window::keyDown(int key) const{
		return m_window && glfwGetKey(m_window, key) == GLFW_PRESS;
	}

	bool Window::mouseDown(int button) const {
		return m_window && glfwGetMouseButton(m_window, button);
	}

	void Window::mouseDelta(double& dx, double& dy) const {
		dx = m_mouseDX;
		dy = m_mouseDY;
	}

	double Window::scrollDeltaY() const {
		return m_scrollY;
	}

}