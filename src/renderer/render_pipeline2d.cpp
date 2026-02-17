#include "renderer/render_pipeline2d.h"
#include "systems/render_system2d.h"
#include "renderer/render_frame2d.h"
#include <cassert>

namespace argon {
	

	void RenderPipeline2D::execute(RenderFrame2D& frame, Renderer& renderer) {
		assert(m_renderSys && "RenderPipeline2D: render system not set");
		assert(frame.scene && frame.cam && frame.matlib && "RenderFrame2D context missing");
		
		int worldPassCount = 0;
		int packetConsumers = 0;

		for (auto& p : m_passes) {
			if (p->needsWorld()) worldPassCount++;
			if (p->needsWorldPackets()) packetConsumers++;
		}

		const bool needRecord = (packetConsumers > 0) || (worldPassCount >= 2);
		frame.mode = needRecord ? FrameMode::Record : FrameMode::Direct;
		
		frame.clearPackets();
		if (frame.mode == FrameMode::Record) {
			m_renderSys->buildPackets(*frame.scene, renderer, *frame.cam, frame.aspect, frame);
		}

		for (auto& pass : m_passes) {
			pass->execute(frame, renderer);
		}

	}

}