#include "scene/scene.h"
#include "renderer/renderer.h"
#include "systems/camera_system.h"
#include "systems/movement_system.h"
#include "systems/render_system2d.h"
#include "renderer/render_frame2d.h"

namespace argon {
	Scene::Scene() 
		: m_moveSys(std::make_unique<MovementSystem>()),
		  m_camSys(std::make_unique<CameraSystem>()),
		  m_renderSys(std::make_unique<RenderSystem2D>()) {}

	Scene::~Scene() = default;
	Scene::Scene(Scene&&) noexcept = default;
	Scene& Scene::operator=(Scene&&) noexcept = default;

	void Scene::update(float dt, FrameContext& ctx)	{
		m_moveSys->update(*this, ctx.window, ctx.input, dt);
		m_camSys->update(ctx.camCtl, dt);
	}

}
