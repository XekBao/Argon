#pragma once
#include <memory>
#include <vector>
#include <cassert>
#include "renderer/render_pass2d.h"

namespace argon {
	
	class RenderFrame2D;
	class RenderSystem2D;

	class RenderPipeline2D {
	public:

		RenderPipeline2D() = default;

		void addPass(std::unique_ptr<RenderPass2D> pass) {
			m_passes.push_back(std::move(pass));
		}

		explicit RenderPipeline2D(RenderSystem2D* renderSys)
			: m_renderSys(renderSys) {
		}

		void setRenderSystem(RenderSystem2D* renderSys) {
			m_renderSys = renderSys;
		}

		void execute(RenderFrame2D& frame, Renderer& renderer);

	private:
		std::vector<std::unique_ptr<RenderPass2D>> m_passes;
		RenderSystem2D* m_renderSys = nullptr;
	};
}