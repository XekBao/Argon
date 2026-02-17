#pragma once
#include <glad/glad.h>
#include <vector>

namespace argon {
	class Mesh {
	public:
		// vertices: [x,y,u,v, x,y,u,v, ...]
		explicit Mesh(const std::vector<float>& vertices);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;

		Mesh(Mesh&&) = delete;
		Mesh& operator=(Mesh&&) = delete;

		void bind() const;
		unsigned int vao() const { return m_vao; }
		int vertexCount() const { return m_vertexCount; }

	private:
		GLuint m_vao = 0;
		GLuint m_vbo = 0;
		int m_vertexCount = 0;
	};
}