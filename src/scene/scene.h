#pragma once
#include <vector>
#include <memory>

#include "scene/entity.h"
#include "scene/frame_context.h"

namespace argon{

	class Renderer;
	class MovementSystem;
	class CameraSystem;
	class RenderSystem2D;
	class Camera2D;
	class RenderFrame2D;

	class Scene {
	public:
		Scene();
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		Scene(Scene&&) noexcept;
		Scene& operator=(Scene&&) noexcept;

		std::vector<Entity> entities;

		void update(float dt, FrameContext& ctx);

	private:
		std::unique_ptr<MovementSystem> m_moveSys;
		std::unique_ptr<CameraSystem>	m_camSys;
		std::unique_ptr<RenderSystem2D>	m_renderSys;
	};
}