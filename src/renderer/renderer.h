#pragma once
#include <vector>
#include <cstdint>
#include "math/mat4.h"
#include "gfx/camera2d.h"
#include "renderer/mesh.h"
#include "renderer/material2d.h"
#include "renderer/render_packet2d.h"
#include "renderer/render_state_cache.h"
#include "renderer/sprite_batcher.h"
#include "renderer/texture_atlas.h"

namespace argon {
	class MaterialLibrary;

	class Renderer {
	public:

		struct Stats {
			std::uint32_t queueCommands = 0;
			std::uint32_t drawCalls = 0;
			std::uint32_t shaderBinds = 0;
			std::uint32_t textureBinds = 0;
			std::uint32_t vaoBinds = 0;
			std::uint32_t batchFlushes = 0;
			std::uint32_t batchedVerts = 0;
			std::uint32_t batchedSprites = 0;

			void reset() { *this = Stats{}; }
		};

		struct PassContext2D {
			Mat4 PV = Mat4::identity();
			const MaterialLibrary* matlib = nullptr;
		};

		const Stats& stats() const { return m_stats; }
		const TextureAtlas* atlas() const { return m_atlas; }

		void clear(float r, float g, float b, float a) const;

		void beginPass(const PassContext2D& ctx);
		void endPass();
		void submit(const RenderPacket2D& pkt);
		
		void setAtlas(const TextureAtlas* atlas) { m_atlas = atlas; }
		void setSpriteQuad(const Mesh* quad) { m_spriteBatcher.setSpriteQuad(quad); }
		void setInstancedSpriteShader(const Shader* s) { m_spriteBatcher.setInstancedSpriteShader(s); }

	private:
		struct RenderCommand {
			const Mesh* mesh = nullptr;
			Mat4 model = Mat4::identity();
			MaterialHandle material = {};
			std::uint32_t layer = 0;
			std::uint64_t key = 0;
			Vec4 tint{ 1.0f,1.0f,1.0f,1.0f };
			Vec4 uvRect{ 0.0f,0.0f,1.0f,1.0f }; // (u0,v0,u1,v1)
		};

		struct SortKey {
			std::uint32_t shaderId = 0;
			std::uint32_t textureId = 0;
			std::uint32_t vaoId = 0;
			std::uint64_t pack() const {
				std::uint64_t key = 0;
				key |= (std::uint64_t(shaderId) << 32);
				key |= (std::uint64_t(textureId) << 16);
				key |= (std::uint64_t(vaoId));
				return key;
			}
		};

		struct BatchVertex { float x, y, u, v; };

		std::uint64_t makeSortKey(const Mesh& mesh, const Material2D& material) const;

		void flush();
		void drawNonBatch(const RenderCommand& cmd, RenderStateCache& st);

	private:
		Stats m_stats{};
		Mat4 m_PV{};

		// legacy vertex batch (not used currently)
		bool m_inScene = false;
		bool m_batchInited = false;
		bool m_hasVertexBatch = false;

		std::vector<RenderCommand> m_queue;
		std::vector<BatchVertex> m_batchVerts;

		std::size_t m_batchVboCapacityVerts = 0; // how many BatchVertex can be contained in current VBO
		unsigned int m_batchVAO = 0;
		unsigned int m_batchVBO = 0;
		Material2D m_vertexBatchMaterial{};
		SpriteBatcher m_spriteBatcher;

		const TextureAtlas* m_atlas = nullptr;
		const MaterialLibrary* m_matlib = nullptr;
	};
}