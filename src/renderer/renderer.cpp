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
		m_batchVerts.clear();
		m_hasVertexBatch = false;

		// hook stats into batcher
		SpriteBatcher::StatsSink sink{};
		sink.drawCalls = &m_stats.drawCalls;
		sink.shaderBinds = &m_stats.shaderBinds;
		sink.textureBinds = &m_stats.textureBinds;
		sink.vaoBinds = &m_stats.vaoBinds;
		sink.batchFlushes = &m_stats.batchFlushes;
		sink.batchedVerts = &m_stats.batchedVerts;

		m_spriteBatcher.begin(m_PV, sink);
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
		cmd.tint = pkt.tint;
		cmd.uvRect = pkt.uvRect;

		cmd.key = makeSortKey(*cmd.mesh, *mat);
		m_queue.push_back(cmd);
		m_stats.queueCommands++;
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

		std::uint64_t currentKey = 0;
		bool hasKey = false;

		// note: m_hasBatchMaterial indicates we have an active instancing batch (with m_batchMaterial set)

		for (const auto& cmd : m_queue) {
			if (!cmd.mesh) continue;

			const Material2D* material = getMatCached(cmd.material);
			if (!material || !material->shader) continue;

			const bool canInst = m_spriteBatcher.canInstance(cmd.mesh, *material);

			if (!canInst) {
				// flush any pending instanced sprites before non-batch draw
				m_spriteBatcher.flush(st);
				drawNonBatch(cmd, st);
				hasKey = false;
				continue;
			}

			// ---- instancing path ----
			// If starting a batch, set batch material+key
			if (!hasKey) {
				currentKey = cmd.key;
				hasKey = true;
			}
			else if (cmd.key != currentKey) {
				m_spriteBatcher.flush(st);
				currentKey = cmd.key;
			}

			m_spriteBatcher.submit(cmd.key, *material, cmd.model, cmd.tint, cmd.uvRect);
		}
		// flush remaining instanced sprites
		m_spriteBatcher.flush(st);
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
		shader.setVec4("uColor", material->color.r * cmd.tint.r,
+							 material->color.g * cmd.tint.g,
+							 material->color.b * cmd.tint.b,
+							 material->color.a * cmd.tint.a);
		shader.setVec4("uUVRect", cmd.uvRect.r, cmd.uvRect.g, cmd.uvRect.b, cmd.uvRect.a);

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
}