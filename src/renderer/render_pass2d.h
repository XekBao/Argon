#pragma once
#include <memory>
#include "renderer/renderer.h"
#include "renderer/render_frame2d.h"

namespace argon {

	class RenderSystem2D;

	class RenderPass2D {
	public:
		virtual ~RenderPass2D() = default;
		virtual void execute(const RenderFrame2D& frame, Renderer& render) = 0;
		virtual bool needsWorld() const { return false; }
		virtual bool needsWorldPackets() const { return false; }
	};

	class WorldPass2D :public RenderPass2D {
	public:
		explicit WorldPass2D(const RenderSystem2D& rs) : m_rs(rs) {}
		bool needsWorld() const override { return true; }
		bool needsWorldPackets() const override { return false; }
		void execute(const RenderFrame2D& frame, Renderer& renderer) override;
	
	private:
		const RenderSystem2D& m_rs;
	};
}