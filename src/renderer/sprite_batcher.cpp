#include "renderer/sprite_batcher.h"
#include <glad/glad.h>
#include <cstring>

#include "renderer/shader.h"
#include "renderer/mesh.h"
#include "renderer/texture2d.h"

namespace argon {
	
	static constexpr std::size_t kMaxBatchedSprites = 20000;
	void SpriteBatcher::begin(const Mat4& PV, StatsSink sink) {
		m_PV = PV;
		m_sink = sink;
		m_instances.clear();
		m_hasBatch = false;
		m_batchKey = 0;
	}

	bool SpriteBatcher::canInstance(const Mesh* mesh, const Material2D& material) const {
		return (m_spriteQuad && mesh == m_spriteQuad) &&
			   (m_instancedSpriteShader && material.shader == m_instancedSpriteShader);
	}

	void SpriteBatcher::submit(std::uint64_t key, const Material2D& material, const Mat4& model) {
		// start
		if (!m_hasBatch) {
			m_hasBatch = true;
			m_batchKey = key;
			m_batchMaterial = material;
		}
		// key changed => flush then start new batch
		InstanceData inst{};
		const Mat4& M = model;
		std::memcpy(inst.m0, &M.m[0], 4 * sizeof(float));
		std::memcpy(inst.m1, &M.m[4], 4 * sizeof(float));
		std::memcpy(inst.m2, &M.m[8], 4 * sizeof(float));
		std::memcpy(inst.m3, &M.m[12], 4 * sizeof(float));

		inst.color[0] = material.color.r;
		inst.color[1] = material.color.g;
		inst.color[2] = material.color.b;
		inst.color[3] = material.color.a;

		m_instances.push_back(inst);
	}

	void SpriteBatcher::flush(RenderStateCache& st) {
		if (!m_hasBatch) return;
		if (m_instances.empty()) { m_hasBatch = false; return; }
		if (!m_batchMaterial.shader) { m_instances.clear(); m_hasBatch = false; return; }
		flushInternal(st);
		m_instances.clear();
		m_hasBatch = false;

	}

	void SpriteBatcher::initInstancingGL() {
		if (m_inited) return;
		static const float quadVerts[] = {
			// x,y, u,v
			-0.5f,-0.5f, 0.f,0.f,
			 0.5f,-0.5f, 1.f,0.f,
			 0.5f, 0.5f, 1.f,1.f,
			-0.5f,-0.5f, 0.f,0.f,
			 0.5f, 0.5f, 1.f,1.f,
			-0.5f, 0.5f, 0.f,1.f,
		};

		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		glGenBuffers(1, &m_quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		glGenBuffers(1, &m_instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);

		m_capacity = kMaxBatchedSprites;
		glBufferData(GL_ARRAY_BUFFER,
			(GLsizeiptr)(m_capacity * sizeof(InstanceData)),
			nullptr,
			GL_DYNAMIC_DRAW);

		const GLsizei stride = (GLsizei)sizeof(InstanceData);

		for (int i = 0; i < 4; ++i) {
			glEnableVertexAttribArray(2 + i);
			glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, stride, (void*)(i * 4 * sizeof(float)));
			glVertexAttribDivisor(2 + i, 1);
		}

		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride, (void*)(16 * sizeof(float)));
		glVertexAttribDivisor(6, 1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_inited = true;
	}

	void SpriteBatcher::flushInternal(RenderStateCache& st) {
		initInstancingGL();
		const Material2D& material = m_batchMaterial;
		const Shader& shader = *material.shader;
		const std::uint32_t shaderId = (std::uint32_t)shader.id();
		if (shaderId != st.shaderId) {
			shader.use();
			shader.setInt("uTex", 0);
			st.shaderId = shaderId;
			if (m_sink.shaderBinds) (*m_sink.shaderBinds)++;
		}

		shader.setMat4("uPV", m_PV.m);

		const bool wantTex = (material.useTexture && material.texture);
		shader.setInt("uUseTex", wantTex ? 1 : 0);

		const std::uint32_t texId = wantTex ? (std::uint32_t)material.texture->id() : 0;
		if (wantTex) {
			if (texId != st.textureId) {
				material.texture->bind(0);
				st.textureId = texId;
				if (m_sink.textureBinds) (*m_sink.textureBinds)++;
			}
		}
		else {
			if (st.textureId != 0) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);
				st.textureId = 0;
				if (m_sink.textureBinds) (*m_sink.textureBinds)++;
			}
		}

		const std::uint32_t vaoId = (std::uint32_t)m_vao;
		if (vaoId != st.vaoId) {
			glBindVertexArray(m_vao);
			st.vaoId = vaoId;
			if (m_sink.vaoBinds) (*m_sink.vaoBinds)++;
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);

		const std::size_t needed = m_instances.size();
		if (needed > m_capacity) {
			while (needed > m_capacity) m_capacity *= 2;
			glBufferData(GL_ARRAY_BUFFER,
				(GLsizeiptr)(m_capacity * sizeof(InstanceData)),
				nullptr,
				GL_DYNAMIC_DRAW);
		}
		else {
			glBufferData(GL_ARRAY_BUFFER,
				(GLsizeiptr)(m_capacity * sizeof(InstanceData)),
				nullptr,
				GL_DYNAMIC_DRAW);
		}

		glBufferSubData(GL_ARRAY_BUFFER,
			0,
			(GLsizeiptr)(needed * sizeof(InstanceData)),
			m_instances.data());

		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)needed);

		if (m_sink.drawCalls) (*m_sink.drawCalls)++;
		if (m_sink.batchFlushes) (*m_sink.batchFlushes)++;
		if (m_sink.batchedVerts) (*m_sink.batchedVerts) += (std::uint32_t)(needed * 6);
	}

}