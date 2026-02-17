#pragma once
#include "renderer/render_pass2d.h"

namespace argon {
	
	class ImGuiPass2D : public RenderPass2D {
	public:
		void execute(const RenderFrame2D& frame, Renderer& renderer) override;
		bool needsWorld() const override { return false; }
		bool needsWorldPackets() const override { return false; }
	};
}