#pragma once
#include <vector>
#include <cstdint>
#include "math/mat4.h"
#include "gfx/camera2d.h"
#include "renderer/mesh.h"
#include "renderer/material2d.h"
#include "renderer/render_packet2d.h"

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

		void clear(float r, float g, float b, float a) const;

		void beginPass(const PassContext2D& ctx);
		void endPass();
		void submit(const RenderPacket2D& pkt);
		
		void setSpriteQuad(const Mesh* quad) { m_spriteQuad = quad; }
		void setInstancedSpriteShader(const Shader* s) { m_instancedSpriteShader = s; }

	private:
		struct RenderCommand {
			const Mesh* mesh = nullptr;
			Mat4 model = Mat4::identity();
			MaterialHandle material = {};
			std::uint32_t layer = 0;
			std::uint64_t key = 0;
		};

		// Store the previous binding object to avoid repeated binding
		struct RenderStateCache {
			std::uint32_t shaderId = 0;
			std::uint32_t textureId = 0;
			std::uint32_t vaoId = 0;
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

		struct InstanceData {
			float m0[4];
			float m1[4];
			float m2[4];
			float m3[4];
			float color[4];
		};

		struct BatchVertex { float x, y, u, v; };

		std::uint64_t makeSortKey(const Mesh& mesh, const Material2D& material) const;

		void flush();
		void drawNonBatch(const RenderCommand& cmd, RenderStateCache& st);
		void initBatchGL();
		void flushBatch(RenderStateCache& st);
		void initInstancingGL();
		void flushInstances(RenderStateCache& st);

	private:
		Stats m_stats{};
		Mat4 m_PV{};

		bool m_inScene = false;
		bool m_batchInited = false;
		bool m_instInited = false;
		bool m_hasBatchMaterial = false;

		std::vector<RenderCommand> m_queue;
		std::vector<BatchVertex> m_batchVerts;
		std::vector<InstanceData> m_instances;

		unsigned int m_instVAO = 0;
		unsigned int m_instQuadVBO = 0;
		unsigned int m_instInstanceVBO = 0;

		std::size_t m_instVboCapacityInstances = 0;
		std::size_t m_batchVboCapacityVerts = 0; // how many BatchVertex can be contained in current VBO
		unsigned int m_batchVAO = 0;
		unsigned int m_batchVBO = 0;

		const Shader* m_instancedSpriteShader = nullptr;
		const Mesh* m_spriteQuad = nullptr;
		const MaterialLibrary* m_matlib = nullptr;
		Material2D m_batchMaterial{};
	};
}