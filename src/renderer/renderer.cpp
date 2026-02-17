#include "renderer.h"
#include <glad/glad.h>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <cassert>
#include "renderer/material_library.h"

namespace argon {
	static constexpr std::size_t kMaxBatchedSprites = 20000;
	static constexpr std::size_t kVertsPerSprite = 6;
	static constexpr std::size_t kMaxBatchVerts = kMaxBatchedSprites * kVertsPerSprite;

	static inline void mulMVP_xy(const float* m, float x, float y, float& outX, float& outY) {
		//Mat4 * vec4(x, y, 0, 1)
		//clip = M * [x y 0 1]^T
		outX = m[0] * x + m[4] * y + m[12];
		outY = m[1] * x + m[5] * y + m[13];
	}
	
	std::uint64_t Renderer::makeSortKey(const Mesh& mesh, const Material2D& material) const {
		SortKey k{};

		const Shader* shader = material.shader;

		k.shaderId = shader ? (std::uint32_t)shader->id() : 0;

		const bool wantTex = (material.useTexture && material.texture);
		k.textureId = wantTex ? (std::uint32_t)material.texture->id() : 0;

		k.vaoId = (std::uint32_t)mesh.vao();

		return k.pack();
	}

	void Renderer::clear(float r, float g, float b, float a) const {
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Renderer::beginPass(const PassContext2D& ctx) {
		assert(ctx.matlib && "PassContext2D.matlib is null");
		m_stats.reset();
		m_PV = ctx.PV;
		m_matlib = ctx.matlib;
		m_inScene = true;

		m_queue.clear();
		m_instances.clear();
		m_batchVerts.clear();
		m_hasBatchMaterial = false;
	}

	void Renderer::endPass() {
		if (!m_inScene) return;
		flush();
		m_inScene = false;
		m_matlib = nullptr;
	}

	void Renderer::submit(const RenderPacket2D& pkt) {
		if (!m_inScene) return;
		if (!pkt.visible) return;
		if (!pkt.mesh) return;
		if (!m_matlib) return;
		//Get Material2D through MaterialLibrary
		const Material2D* mat = m_matlib ? m_matlib->get(pkt.material) : nullptr;
		if (!mat || !mat->shader) return;

		RenderCommand cmd;
		cmd.mesh = pkt.mesh;
		cmd.model = pkt.model;
		cmd.material = pkt.material;
		cmd.layer = (std::uint32_t)pkt.layer;

		cmd.key = makeSortKey(*cmd.mesh, *mat);
		m_queue.push_back(cmd);
		m_stats.queueCommands++;
	}

	void Renderer::initBatchGL() {
		if (m_batchInited) return;

		glGenVertexArrays(1, &m_batchVAO);
		glGenBuffers(1, &m_batchVBO);

		glBindVertexArray(m_batchVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_batchVBO);

		//Data not given here. The data is given when flushBatch
		m_batchVboCapacityVerts = kMaxBatchVerts;
		glBufferData(GL_ARRAY_BUFFER,
					(GLsizeiptr)(m_batchVboCapacityVerts * sizeof(BatchVertex)),
					nullptr,
					GL_DYNAMIC_DRAW);

		//layout(location=0) vec2 aPos;
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (void*)0);
		glEnableVertexAttribArray(0);
		//layout(location=1) vec2 aUV;
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_batchInited = true;
	}

	void Renderer::flush() {
		if (m_queue.empty()) return;
		if (!m_matlib) { m_queue.clear(); return; }

		std::sort(m_queue.begin(), m_queue.end(),
			[](const RenderCommand& a, const RenderCommand& b) {
				if (a.layer != b.layer) return a.layer < b.layer;
				return a.key < b.key;
			}
		);

		RenderStateCache st{};

		// ---- local cache: avoid calling matlib->get for every cmd ----
		MaterialHandle lastH{};                 // depends on your handle type; default {} ok
		const Material2D* cachedMat = nullptr;  // cached pointer for lastH
		bool hasCached = false;

		auto getMatCached = [&](MaterialHandle h) -> const Material2D* {
			if (hasCached && h == lastH) return cachedMat;
			lastH = h;
			cachedMat = m_matlib->get(h);
			hasCached = true;
			return cachedMat;
			};

		std::uint64_t batchKey = 0;
		// note: m_hasBatchMaterial indicates we have an active instancing batch (with m_batchMaterial set)

		for (const auto& cmd : m_queue) {
			if (!cmd.mesh) continue;

			const Material2D* material = getMatCached(cmd.material);
			if (!material || !material->shader) continue;

			const bool canInstance =
				(m_spriteQuad && cmd.mesh == m_spriteQuad) &&
				(m_instancedSpriteShader && material->shader == m_instancedSpriteShader);

			if (!canInstance) {
				// Before drawing a non-instanced command, flush any pending instances
				flushInstances(st);
				drawNonBatch(cmd, st);
				continue;
			}

			// ---- instancing path ----
			// If starting a batch, set batch material+key
			if (!m_hasBatchMaterial) {
				m_batchMaterial = *material;
				m_hasBatchMaterial = true;
				batchKey = cmd.key;
			}
			// If key changed, flush previous batch, start new one
			else if (cmd.key != batchKey) {
				flushInstances(st);             // flushInstances will set m_hasBatchMaterial=false
				m_batchMaterial = *material;    // start new batch
				m_hasBatchMaterial = true;
				batchKey = cmd.key;
			}

			// push instance data
			const Mat4& M = cmd.model;
			InstanceData inst{};
			std::memcpy(inst.m0, &M.m[0], 4 * sizeof(float));
			std::memcpy(inst.m1, &M.m[4], 4 * sizeof(float));
			std::memcpy(inst.m2, &M.m[8], 4 * sizeof(float));
			std::memcpy(inst.m3, &M.m[12], 4 * sizeof(float));

			inst.color[0] = material->color.r;
			inst.color[1] = material->color.g;
			inst.color[2] = material->color.b;
			inst.color[3] = material->color.a;

			m_instances.push_back(inst);
		}

		flushInstances(st);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		m_queue.clear();
	}

	void Renderer::drawNonBatch(const RenderCommand& cmd, RenderStateCache& st) {
		if (!m_matlib) return;

		const Material2D* material = m_matlib->get(cmd.material);
		if (!material || !material->shader) return;

		const Shader& shader = *material->shader;
		const Mesh& mesh = *cmd.mesh;
		
		const std::uint32_t shaderId = (std::uint32_t)shader.id();
		if (shaderId != st.shaderId) {
			shader.use();
			// TODO here assume the texture is always in 0
			shader.setInt("uTex", 0);
			st.shaderId = shaderId;
			m_stats.shaderBinds++;
		}

		//MVP
		Mat4 MVP = mul(m_PV, cmd.model);
		shader.setMat4("uMVP", MVP.m);
		shader.setVec4("uColor", material->color.r,
								 material->color.g,
								 material->color.b,
								 material->color.a);

		const bool wantTex = (material->useTexture && material->texture);
		shader.setInt("uUseTex", wantTex ? 1 : 0);
		const std::uint32_t texId = wantTex ? (std::uint32_t)material->texture->id() : 0;
		if (wantTex) {
			if (texId != st.textureId) {
				material->texture->bind(0);
				st.textureId = texId;
				m_stats.textureBinds++;
			}
		} else {
			if (st.textureId != 0) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);
				st.textureId = 0;
				m_stats.textureBinds++;
			}
		}

		const std::uint32_t vaoId = (std::uint32_t)mesh.vao();
		if (vaoId != st.vaoId) {
			mesh.bind();
			st.vaoId = vaoId;
			m_stats.vaoBinds++;
		}

		glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount());
		m_stats.drawCalls++;
	}

	void Renderer::flushBatch(RenderStateCache& st) {
		if (!m_hasBatchMaterial || m_batchVerts.empty()) return;
		if (!m_batchMaterial.shader) return;

		initBatchGL();

		const Material2D& material = m_batchMaterial;
		const Shader& shader = *material.shader;

		static const float I[16] = {
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1
		};

		const std::uint32_t shaderId = (std::uint32_t)shader.id();
		if (shaderId != st.shaderId) {
			shader.use();
			shader.setInt("uTex", 0);
			st.shaderId = shaderId;
			m_stats.shaderBinds++;
		}

		shader.setMat4("uMVP", I);
		shader.setVec4("uColor", material.color.r,
								 material.color.g,
								 material.color.b,
								 material.color.a);

		const bool wantTex = (material.useTexture && material.texture);
		shader.setInt("uUseTex", wantTex ? 1 : 0);

		const std::uint32_t texId = wantTex ? (std::uint32_t)material.texture->id() : 0;
		if (wantTex) {
			if (texId != st.textureId) {
				material.texture->bind(0);
				st.textureId = texId;
				m_stats.textureBinds++;
			}
		} else {
			if (st.textureId != 0) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);
				st.textureId = 0;
				m_stats.textureBinds++;
			}
		}

		const std::uint32_t vaoId = (std::uint32_t)m_batchVAO;
		if (vaoId != st.vaoId) {
			glBindVertexArray(m_batchVAO);
			st.vaoId = vaoId;
			m_stats.vaoBinds++;
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_batchVBO);

		const std::size_t neededVerts = m_batchVerts.size();

		if (neededVerts > m_batchVboCapacityVerts) {
			while (neededVerts > m_batchVboCapacityVerts)
				m_batchVboCapacityVerts *= 2;

			#ifdef _DEBUG
			if (neededVerts > m_batchVboCapacityVerts) {
				std::cerr << "[Renderer] Batch VBO capacity logic error\n";
				std::abort();
			}
			#endif

			glBufferData(GL_ARRAY_BUFFER,
				(GLsizeiptr)(m_batchVboCapacityVerts * sizeof(BatchVertex)),
				nullptr,
				GL_DYNAMIC_DRAW);
		} else {
			// orphan
			glBufferData(GL_ARRAY_BUFFER,
				(GLsizeiptr)(m_batchVboCapacityVerts * sizeof(BatchVertex)),
				nullptr,
				GL_DYNAMIC_DRAW);
		}

		glBufferSubData(GL_ARRAY_BUFFER,
			0,
			(GLsizeiptr)(neededVerts * sizeof(BatchVertex)),
			m_batchVerts.data());

		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_batchVerts.size());
		

		//stats
		m_stats.drawCalls++;
		m_stats.batchFlushes++;
		m_stats.batchedVerts += (std::uint32_t)m_batchVerts.size();

		m_batchVerts.clear();
		m_hasBatchMaterial = false;
	}

	void Renderer::initInstancingGL() {
		if (m_instInited) return;
		
		static const float quadVerts[] = {
			// x,y, u,v
			-0.5f,-0.5f, 0.f,0.f,
			 0.5f,-0.5f, 1.f,0.f,
			 0.5f, 0.5f, 1.f,1.f,
			-0.5f,-0.5f, 0.f,0.f,
			 0.5f, 0.5f, 1.f,1.f,
			-0.5f, 0.5f, 0.f,1.f,
		};

		glGenVertexArrays(1, &m_instVAO);
		glBindVertexArray(m_instVAO);
		//quad vertex VBO (static)
		glGenBuffers(1, &m_instQuadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_instQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
		// instance VBO (dynamic)
		glGenBuffers(1, &m_instInstanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_instInstanceVBO);
		
		m_instVboCapacityInstances = kMaxBatchedSprites;
		glBufferData(GL_ARRAY_BUFFER,
			(GLsizeiptr)(m_instVboCapacityInstances * sizeof(InstanceData)),
			nullptr,
			GL_DYNAMIC_DRAW);

		const GLsizei stride = (GLsizei)sizeof(InstanceData);

		// iM0...iM3, location 2, 3, 4, 5
		for (int i = 0;i < 4;++i) {
			glEnableVertexAttribArray(2 + i);
			glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, stride, (void*)(i * 4 * sizeof(float)));
			glVertexAttribDivisor(2 + i, 1);
		}
		// iColor, location 6
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride, (void*)(16 * sizeof(float)));
		glVertexAttribDivisor(6, 1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_instInited = true;
	}

	void Renderer::flushInstances(RenderStateCache& st) {
		if (!m_hasBatchMaterial) return;

		if (m_instances.empty()) {
			m_hasBatchMaterial = false;
			return;
		}

		if (!m_batchMaterial.shader) {
			m_instances.clear();
			m_hasBatchMaterial = false;
			return;
		}

		initInstancingGL();

		const Material2D& material = m_batchMaterial;
		const Shader& shader = *material.shader;
		// bind shader
		const std::uint32_t shaderId = (std::uint32_t)shader.id();
		if (shaderId != st.shaderId) {
			shader.use();
			shader.setInt("uTex", 0);
			st.shaderId = shaderId;
			m_stats.shaderBinds++;
		}
		//per-flush PV
		shader.setMat4("uPV", m_PV.m);

		const bool wantTex = (material.useTexture && material.texture);
		shader.setInt("uUseTex", wantTex ? 1 : 0);
		const std::uint32_t texId = wantTex ? (std::uint32_t)material.texture->id() : 0;
		if (wantTex) {
			if (texId != st.textureId) {
				material.texture->bind(0);
				st.textureId = texId;
				m_stats.textureBinds++;
			}
		} else {
			if (st.textureId != 0) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);
				st.textureId = 0;
				m_stats.textureBinds++;
			}
		}
		//VAO bind
		const std::uint32_t vaoId = (std::uint32_t)m_instVAO;
		if (vaoId != st.vaoId) {
			glBindVertexArray(m_instVAO);
			st.vaoId = vaoId;
			m_stats.vaoBinds++;
		}
		//upload instances
		glBindBuffer(GL_ARRAY_BUFFER, m_instInstanceVBO);
		const std::size_t neededInstSize = m_instances.size();
		if (neededInstSize > m_instVboCapacityInstances) {
			while (neededInstSize > m_instVboCapacityInstances) m_instVboCapacityInstances *= 2;
			glBufferData(GL_ARRAY_BUFFER,
				(GLsizeiptr)(m_instVboCapacityInstances * sizeof(InstanceData)),
				nullptr,
				GL_DYNAMIC_DRAW);
		} else {
			// orphan
			glBufferData(GL_ARRAY_BUFFER,
				(GLsizeiptr)(m_instVboCapacityInstances * sizeof(InstanceData)),
				nullptr,
				GL_DYNAMIC_DRAW);
		}

		glBufferSubData(GL_ARRAY_BUFFER,
			0,
			(GLsizeiptr)(neededInstSize * sizeof(InstanceData)),
			m_instances.data());

		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)m_instances.size());
		m_stats.drawCalls++;
		m_stats.batchFlushes++;
		m_stats.batchedVerts += (std::uint32_t)(m_instances.size() * 6);

		m_instances.clear();
		m_hasBatchMaterial = false;
	}
	
}