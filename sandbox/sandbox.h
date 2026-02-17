#pragma once
#include <memory>
#include "platform/window.h"
#include "renderer/shader.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "gfx/camera2d.h"
#include "gfx/camera_controller2d.h"
#include "math/transform.h"
#include "scene/scene.h"
#include "input/input_map.h"
#include "systems/movement_system.h"
#include "systems/camera_system.h"
#include "systems/render_system2d.h"
#include "renderer/material_library.h"
#include "renderer/material_handle.h"
#include "renderer/render_pipeline2d.h"
#include "renderer/render_pass2d.h"
#include "renderer/render_frame2d.h"
#include "renderer/imgui_pass2d.h"
#include "renderer/texture_atlas.h"

namespace argon {
	class SandboxApp {
	public:
		int run();
	private:
		bool init();
		void shutdown();

		void update(float dt);
		void render();

	private:
		std::unique_ptr<Window> m_window;

		std::unique_ptr<Shader> m_basicShader;
		std::unique_ptr<Shader> m_spriteShader;

		std::unique_ptr<Mesh> m_tri;
		std::unique_ptr<Mesh> m_quad;
		std::unique_ptr<CameraController2D> m_camCtl;
		std::unique_ptr<Texture2D> m_testTex;
		std::unique_ptr<Texture2D> m_atlasTex;

		std::vector<std::unique_ptr<Texture2D>> m_textures;

		std::vector<MaterialHandle> m_matHandles;

		Renderer m_renderer;
		Camera2D m_camera;
		Scene m_scene;
		InputMap m_input;
		MovementSystem m_moveSys;
		CameraSystem m_camSys;
		MaterialLibrary m_materials;
		MaterialHandle m_matTex = 0;
		MaterialHandle m_matColor = 0;
		RenderPipeline2D m_pipeline2d;
		RenderFrame2D    m_frame2d;
		RenderSystem2D m_renderSys;
		TextureAtlas m_atlas;

		double m_lastTime = 0.0;
		double m_time = 0.0;

		float m_pulse = 0.0f;
	};
}