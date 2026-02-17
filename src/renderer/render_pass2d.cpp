#include "renderer/render_pass2d.h"
#include "systems/render_system2d.h"
#include <cassert>
#include "renderer/material_library.h"

namespace argon {
	void WorldPass2D::execute(const RenderFrame2D& frame, Renderer& renderer) {
		assert(frame.matlib && "RenderFrame2D.matlib is null");

		Renderer::PassContext2D ctx;
		ctx.PV = frame.PV;
		ctx.matlib = frame.matlib;

		renderer.beginPass(ctx);

		if (frame.mode == FrameMode::Direct) {
			assert(frame.scene && frame.cam && "Direct mode needs scene+cam");
			m_rs.submitVisible(*frame.scene, renderer, *frame.cam, frame.aspect);
		} else {
			for (const auto& pkt : frame.packets) {
				if (!pkt.visible) continue;
				renderer.submit(pkt);
			}
		}

		renderer.endPass();
	}
}