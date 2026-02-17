#pragma once
#include <unordered_map>
#include <glad/glad.h>
#include <cstdint>
#include <string>

namespace argon {

	class Shader {
	public:
		Shader(const char* vsSrc, const char* fsSrc);
		~Shader();

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		void use() const;
		GLuint id() const { return m_program; }

		void setFloat(const char* name, float v) const;
		void setMat4(const char* name, const float* m4) const;
		void setVec4(const char* name, const float r, const float g, const float b, const float a) const;
		void setInt(const char* name, int v)  const;

		std::uint64_t uniformLookups() const { return m_uniformLookups; }
		std::uint64_t uniformGLQueries() const { return m_uniformGLQueries; }

	private:
		mutable std::uint64_t m_uniformLookups = 0; // number of uniformLoc call
		mutable std::uint64_t m_uniformGLQueries = 0; // number of glGetUniformLocation find call

		mutable std::unordered_map<std::string, GLint> m_locCache;
		GLuint m_program = 0;
		GLint uniformLoc(const char* name) const;
		static GLuint compile(GLenum type, const char* src);
		static GLuint link(GLuint vsId, GLuint fsId);
	};
}