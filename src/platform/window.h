#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

namespace argon {
	class Window {
	public:
		Window(int width, int height, const char* title);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		GLFWwindow* handle() const { return m_window; }
		bool shouldClose() const;
		void swapBuffers() const;
		void pollEvents();

		int framebufferWidth() const { return m_fbWidth; }
		int framebufferHeight() const { return m_fbHeight; }

		void setVsync(bool enabled);

		bool keyDown(int key) const;
		bool mouseDown(int button) const;

		void mouseDelta(double& dx, double& dy) const;
		double scrollDeltaY() const;

	private:
		GLFWwindow* m_window = nullptr;
		int m_fbWidth = 0;
		int m_fbHeight = 0;

		double m_mouseX = 0.0, m_mouseY = 0.0;
		double m_mouseDX = 0.0, m_mouseDY = 0.0;
		double m_scrollY = 0.0;

		static void scrollCallback(GLFWwindow* win, double xoff, double yoff);

		static void glfwErrorCallback(int code, const char* desc);
		static void framebufferSizeCallback(GLFWwindow* win, int w, int h);
	};
}