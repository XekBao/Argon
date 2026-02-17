#include "sandbox.h"
#include "math/mat4.h"
#include <cmath>
#include <iostream>
#include <memory>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


namespace argon {
	static const char* vsBasic = R"(
	#version 330 core
	layout (location = 0) in vec2 aPos;
	layout (location = 1) in vec2 aUV;
	
	uniform mat4 uMVP;
	uniform vec4 uUVRect; // (u0,v0,u1,v1)
	out vec2 vUV;
	
	void main() {
	    vUV = mix(uUVRect.xy, uUVRect.zw, aUV);
	    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
	}
	)";
	
	static const char* fsBasic = R"(
	#version 330 core
	out vec4 FragColor;
	in vec2 vUV;
	
	uniform sampler2D uTex;
	uniform int uUseTex;
	uniform vec4 uColor;
	
	void main() {
	    vec4 base = uColor;
	    if (uUseTex == 1) base *= texture(uTex, vUV);
	    FragColor = base;
	}
)";

	static const char* vsInstanced = R"(
	#version 330 core
	layout (location = 0) in vec2 aPos;
	layout (location = 1) in vec2 aUV;
	
	// instance attributes: mat4
	layout (location = 2) in vec4 iM0;
	layout (location = 3) in vec4 iM1;
	layout (location = 4) in vec4 iM2;
	layout (location = 5) in vec4 iM3;

	layout (location = 6) in vec4 iColor;
	layout (location = 7) in vec4 iUVRect; // (u0,v0,u1,v1)

	uniform mat4 uPV;

	out vec2 vUV;
	out vec4 vColor;

	void main() {
		vUV = mix(iUVRect.xy, iUVRect.zw, aUV);
		vColor = iColor;
		mat4 model = mat4(iM0, iM1, iM2, iM3);
		gl_Position = uPV * model * vec4(aPos, 0.0, 1.0);
	}
	)";

	static const char* fsInstanced = R"(
	#version 330 core
	out vec4 FragColor;

	in vec2 vUV;
    in vec4 vColor;	

	uniform sampler2D uTex;
	uniform int uUseTex;

	void main() {
		vec4 base = vColor;
		if (uUseTex == 1) {
			base *= texture(uTex, vUV);
		}
		FragColor = base;
	}
	)";

	bool SandboxApp::init() {

		const int numQuads = 5000;
		const int gridW = 100;   // 100*50 = 5000
		const int gridH = 50;

		m_window = std::make_unique<Window>(800, 600, "Argon");
		if (!m_window || !m_window->handle()) {
			std::cerr << "Failed to create window\n";
			return false;
		}
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cerr << "Failed to initialize GLAD\n";
			return false;
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(m_window->handle(), true);
		ImGui_ImplOpenGL3_Init("#version 330");

		glfwSwapInterval(0);

		std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
		
		m_basicShader = std::make_unique<Shader>(vsBasic, fsBasic);
		m_spriteShader = std::make_unique<Shader>(vsInstanced, fsInstanced);

		if (!m_basicShader->id() || !m_spriteShader->id()) {
			std::cerr << "Failed to create shader program.\n";
			return false;
		}

		m_input.bind(Action::MoveLeft, GLFW_KEY_A);
		m_input.bind(Action::MoveRight, GLFW_KEY_D);
		m_input.bind(Action::MoveUp, GLFW_KEY_W);
		m_input.bind(Action::MoveDown, GLFW_KEY_S);

		const int numTextures = 8;
		m_textures.clear();
		m_textures.reserve(numTextures);
		for (int i = 0; i < numTextures; ++i) {
			std::string path = "assets/texture" + std::to_string(i) + ".jpg";
			m_textures.push_back(std::make_unique<Texture2D>(path.c_str()));
		}

		m_matHandles.clear();
		m_matHandles.reserve(numTextures);

		for (int i = 0; i < numTextures; ++i) {
			Material2D m;
			m.shader = m_spriteShader.get();
			m.texture = m_textures[i].get();
			m.useTexture = true;
			m.color = { 1,1,1,1 };
			m_matHandles.push_back(m_materials.add(m));
		}

		m_tri = std::make_unique<Mesh>(std::vector<float>{
			// x     y     u     v
			-0.6f, -0.4f, 0.0f, 0.0f,
				0.6f, -0.4f, 1.0f, 0.0f,
				0.0f, 0.6f, 0.5f, 1.0f
		});

		m_quad = std::make_unique<Mesh>(std::vector<float>{
			// x,    y,    u,   v
			-0.5f, -0.5f, 0.f, 0.f,
			0.5f, -0.5f, 1.f, 0.f,
			0.5f, 0.5f, 1.f, 1.f,

			-0.5f, -0.5f, 0.f, 0.f,
			0.5f, 0.5f, 1.f, 1.f,
			-0.5f, 0.5f, 0.f, 1.f
		});

		m_renderer.setSpriteQuad(m_quad.get());
		m_renderer.setInstancedSpriteShader(m_spriteShader.get());

		m_camera.size = 1.0f;
		m_camera.zoom = 1.0f;
		m_camera.x = 0.0f;
		m_camera.y = 0.0f;

		m_camCtl = std::make_unique<CameraController2D>(*m_window, m_camera);
		m_testTex = std::make_unique<Texture2D>("assets/texture0.jpg");
		m_atlasTex = std::make_unique<Texture2D>("assets/test_atlas.png");

		m_scene.entities.clear();
		m_scene.entities.reserve(numQuads);

		Material2D matTex;
		matTex.shader = m_basicShader.get();
		matTex.texture = m_testTex.get();
		matTex.useTexture = true;
		matTex.color = { 1.0f,1.0f,1.0f,1.0f };
		m_matTex = m_materials.add(matTex);

		Material2D matAtlas;
		matAtlas.shader = m_spriteShader.get();
		matAtlas.texture = m_atlasTex.get();
		matAtlas.useTexture = true;
		matAtlas.color = { 1,1,1,1 };
		MaterialHandle m_matAtlas = m_materials.add(matAtlas);

		m_atlas.setTextureSize(1024, 1024);
		bool ok = m_atlas.loadFromFile("assets/test.atlas");
		std::cout << "atlas load ok=" << ok << "\n";
		m_renderer.setAtlas(&m_atlas);

		const float spacing = 0.05f;
		const float startX = -(gridW - 1) * spacing * 0.5f;
		const float startY = -(gridH - 1) * spacing * 0.5f;
		static const char* sprites[] = { "hero0","hero1","coin0","coin1" };
		const int spriteCount = 4;

		for (int y = 0; y < gridH; ++y) {
			for (int x = 0; x < gridW; ++x) {
				Entity e;
				e.renderable.mesh = m_quad.get();
				int idx = (x + y * gridW) % numTextures;
				e.renderable.material = m_matAtlas;

				const char* sprName = sprites[(x + y) % spriteCount];
				e.renderable.spriteId = m_atlas.getId(sprName);

				e.transform.x = startX + x * spacing;
				e.transform.y = startY + y * spacing;
				e.transform.sx = 0.05f;   
				e.transform.sy = 0.05f;
				e.renderable.layer = 0;
				e.renderable.tint = { (float)(x % 10) / 9.0f, (float)(y % 10) / 9.0f, 1.0f, 1.0f };
				m_scene.entities.push_back(e);
			}
		}

		Material2D matColor;
		matColor.shader = m_basicShader.get();
		matColor.texture = nullptr;
		matColor.useTexture = false;
		matColor.color = { 0.2f, 0.8f, 0.3f, 1.0f };
		m_matColor = m_materials.add(matColor);

		Entity e1;
		e1.renderable.mesh = m_quad.get();
		e1.renderable.material = m_matTex;
		e1.renderable.layer = 10;
		e1.transform.x = -0.7;
		
		Entity e2;
		e2.renderable.mesh = m_tri.get();
		e2.renderable.material = m_matColor;
		e2.renderable.layer = 10;
		e2.transform.x = 0.7;

		m_scene.entities.push_back(e1);
		m_scene.entities.push_back(e2);

		if (!m_scene.entities.empty())
			m_scene.entities[0].controllable = true;

		m_pipeline2d = RenderPipeline2D{};
		m_pipeline2d.setRenderSystem(& m_renderSys);
		m_pipeline2d.addPass(std::make_unique<WorldPass2D>(m_renderSys));
		m_pipeline2d.addPass(std::make_unique<ImGuiPass2D>());

		m_lastTime = glfwGetTime();
		m_time = m_lastTime;

		return true;
	}

	void SandboxApp::shutdown() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		m_tri.reset();
		m_window.reset();
		m_camCtl.reset();
		m_quad.reset();
		m_testTex.reset();
	} 

	void SandboxApp::update(float dt) {

		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureKeyboard || io.WantCaptureMouse) {
			return;
		}

		m_time += dt;
		m_pulse = 0.5f + 0.5f * std::sin((float)m_time);

		FrameContext ctx{ *m_window, m_input, *m_camCtl, m_materials};
		m_scene.update(dt, ctx);

		if (m_scene.entities.size() >= 2) {
			m_scene.entities[0].transform.rotation = (float)m_time;
			m_scene.entities[1].transform.y = 0.3f * std::sin((float)m_time * 2.0f);
		}
	}

	void SandboxApp::render() {
		int fbW = m_window->framebufferWidth();
		int fbH = m_window->framebufferHeight();
		glViewport(0, 0, fbW, fbH);
		m_renderer.clear(0.15f, 0.18f, 0.25f, 1.0f);
		float aspect = (fbH != 0) ? (float)fbW / (float)fbH : 1.0f;

		m_frame2d.matlib = &m_materials;
		m_frame2d.scene = &m_scene;
		m_frame2d.cam = &m_camera;
		m_frame2d.aspect = aspect;
		m_frame2d.PV = m_camera.projView(aspect);

		m_pipeline2d.execute(m_frame2d, m_renderer);
	}

	int SandboxApp::run() {
		if (!init()) {
			shutdown();
			return -1;
		}

		double lastPrint = 0.0;
		double fpsLast = glfwGetTime();
		int    fpsFrames = 0;

		while (!m_window->shouldClose()) {

			m_window->pollEvents();

			double now = glfwGetTime();
			float dt = (float)(now - m_lastTime);
			m_lastTime = now;

			if (dt > 0.1f) dt = 0.1f;

			update(dt);
			render();

			fpsFrames++;
			double fpsElapsed = now - fpsLast;


			if (now - lastPrint > 0.5) {
				lastPrint = now;
				double fps = 0.0;
				double ms = 0.0;
				if (fpsElapsed > 0.0001) {
					fps = fpsFrames / fpsElapsed;
					ms = 1000.0 / fps;
				}
				const auto& s = m_renderer.stats();

				std::cout
					<< "FPS=" << fps
					<< " ms=" << ms
					<< "queued=" << s.queueCommands
					<< " drawCalls=" << s.drawCalls
					<< " shaderBinds=" << s.shaderBinds
					<< " texBinds=" << s.textureBinds
					<< " vaoBinds=" << s.vaoBinds
					<< " batchFlushes=" << s.batchFlushes
					<< " batchedVerts=" << s.batchedVerts
					<< "\n";

				std::string title =
					"Argon | FPS: " + std::to_string((int)(fps + 0.5)) +
					" | " + std::to_string(ms).substr(0, 5) + " ms" +
					" | draw: " + std::to_string(s.drawCalls) +
					" | flush: " + std::to_string(s.batchFlushes) +
					" | tex: " + std::to_string(s.textureBinds);

				glfwSetWindowTitle(m_window->handle(), title.c_str());
			}
			m_window->swapBuffers();
		}

		shutdown();
		return 0;
	}

}