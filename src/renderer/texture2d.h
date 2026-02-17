#pragma once
#include <glad/glad.h>
#include <string>

namespace argon {

	class Texture2D {

	public:
		explicit Texture2D(const std::string& path);
		~Texture2D();

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator= (const Texture2D&) = delete;

		void bind(int unit = 0) const;

		int width() const { return m_w; }
		int height() const { return m_h; }
		unsigned int id() const { return m_id; }

	private:
		GLuint m_id = 0;
		int m_w = 0, m_h = 0, m_channels = 0;

	};

}