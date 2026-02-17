#pragma once
#include <vector>
#include <memory>

#include "scene/entity.h"
#include "scene/frame_context.h"

namespace argon{

	class MovementSystem;
	class CameraSystem;

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
	};
}