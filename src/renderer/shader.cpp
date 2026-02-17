#include "shader.h"
#include <iostream>
#include <string>

namespace argon {

	GLuint Shader::compile(GLenum type, const char* src) {
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		GLint ok = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
		if (!ok) {
			GLint logLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
			std::string log(logLen, '\0');
			glGetShaderInfoLog(shader, logLen, nullptr, log.data());

			const char* kind = (type == GL_VERTEX_SHADER) ? "VERTEX" :
				(type == GL_FRAGMENT_SHADER) ? "FRAGMENT" : "UNKNOWN";
			std::cerr << "[Shader Compile Error][" << kind << "]\n" << log << "\n";
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}

	GLuint Shader::link(GLuint vsId, GLuint fsId) {
		GLuint prog = glCreateProgram();
		glAttachShader(prog, vsId);
		glAttachShader(prog, fsId);
		glLinkProgram(prog);

		GLint ok = 0;
		glGetProgramiv(prog, GL_LINK_STATUS, &ok);
		if (!ok) {
			GLint logLen = 0;
			glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
			std::string log(logLen, '\0');
			glGetProgramInfoLog(prog, logLen, nullptr, log.data());
			std::cerr << "[Program Link Error ]\n" << log << "\n";
			glDeleteProgram(prog);
			return 0;
		}
		return prog;
	}

	Shader::Shader(const char* vsSrc, const char* fsSrc) {
		GLuint vsId = compile(GL_VERTEX_SHADER, vsSrc);
		if (!vsId) return;

		GLuint fsId = compile(GL_FRAGMENT_SHADER, fsSrc);
		if (!fsId) {
			glDeleteShader(vsId);
			return;
		}
		m_program = link(vsId, fsId);
		m_locCache.clear();
		m_uniformLookups = 0;
		m_uniformGLQueries = 0;
		glDeleteShader(vsId);
		glDeleteShader(fsId);
	}

	Shader::~Shader() {
		if (m_program) {
			glDeleteProgram(m_program);
			m_program = 0;
		}
	}

	void Shader::use() const {
		glUseProgram(m_program);
	}

	GLint Shader::uniformLoc(const char* name) const {

		m_uniformLookups++;
		if (!m_program || !name) return -1;
		// check cache first
		auto it = m_locCache.find(name);
		if (it != m_locCache.end())
			return it->second;
		// if not in Cache, check OpenGL
		m_uniformGLQueries++;
		GLint loc = glGetUniformLocation(m_program, name);

		// put the loc into cache (even cache when loc=-1)
		// to avoid checking uniform don't exist
		m_locCache.try_emplace(name, loc);
		return loc;
	}

	void Shader::setFloat(const char* name, float v) const {
		GLint loc = uniformLoc(name);
		if (loc >= 0) glUniform1f(loc, v);
	}

	void Shader::setMat4(const char* name, const float* m4) const {
		GLint loc = uniformLoc(name);
		if (loc >= 0)glUniformMatrix4fv(loc, 1, GL_FALSE, m4);
	}

	void Shader::setVec4(const char* name, const float r, const float g, const float b, const float a) const {
		GLint loc = uniformLoc(name);
		if (loc >= 0)glUniform4f(loc, r, g, b, a);
	}

	void Shader::setInt(const char* name, int v) const {
		GLint loc = uniformLoc(name);
		if (loc >= 0) glUniform1i(loc, v);
	}

}