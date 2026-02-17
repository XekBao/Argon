#pragma once
#include <vector>
#include <cstdint>
#include "math/mat4.h"
#include "renderer/material2d.h"
#include "renderer/render_state_cache.h"

namespace argon {

	class Mesh;
	class Shader;

	class SpriteBatcher {
	public:
		struct StatsSink {
			std::uint32_t* drawCalls = nullptr;
			std::uint32_t* shaderBinds = nullptr;
			std::uint32_t* textureBinds = nullptr;
			std::uint32_t* vaoBinds = nullptr;
			std::uint32_t* batchFlushes = nullptr;
			std::uint32_t* batchedVerts = nullptr;
		};

		void setSpriteQuad(const Mesh* quad) { m_spriteQuad = quad; }
		void setInstancedSpriteShader(const Shader* s) { m_instancedSpriteShader = s; }

		void begin(const Mat4& PV, StatsSink sink);
		bool canInstance(const Mesh* mesh, const Material2D& material) const;
		void submit(std::uint64_t key, const Material2D& material, const Mat4& model,
					const Vec4& tint, const Vec4& uvRect);

		void flush(RenderStateCache& st);

	private:
		struct InstanceData {
			float m0[4];
			float m1[4];
			float m2[4];
			float m3[4];
			float color[4];
			float uvRect[4];
		};

		void initInstancingGL();
		void flushInternal(RenderStateCache& st);

	private:
		// per-pass
		Mat4 m_PV{};
		StatsSink m_sink{};

		// batching state
		bool m_hasBatch = false;
		std::uint64_t m_batchKey = 0;
		Material2D m_batchMaterial{};
		std::vector<InstanceData> m_instances;

		// GL
		bool m_inited = false;
		unsigned int m_vao = 0;
		unsigned int m_quadVBO = 0;
		unsigned int m_instanceVBO = 0;
		std::size_t m_capacity = 0;

		// matching condition
		const Shader* m_instancedSpriteShader = nullptr;
		const Mesh* m_spriteQuad = nullptr;
	};
}