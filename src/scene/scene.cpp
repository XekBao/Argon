#include "scene/scene.h"
#include "systems/camera_system.h"
#include "systems/movement_system.h"

namespace argon {
	Scene::Scene() 
		: m_moveSys(std::make_unique<MovementSystem>()),
		  m_camSys(std::make_unique<CameraSystem>()){}

	Scene::~Scene() = default;
	Scene::Scene(Scene&&) noexcept = default;
	Scene& Scene::operator=(Scene&&) noexcept = default;

	void Scene::update(float dt, FrameContext& ctx)	{
		m_moveSys->update(*this, ctx.window, ctx.input, dt);
		m_camSys->update(ctx.camCtl, dt);
	}

}
