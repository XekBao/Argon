#include "systems/render_system2d.h"
#include "scene/scene.h"
#include "renderer/renderer.h"
#include "renderer/render_packet2d.h"
#include "renderer/render_frame2d.h"
#include "gfx/camera2d.h"
#include <cmath>
#include <vector>

namespace argon {

	static inline bool aabbIntersects(
		float minAx, float minAy, float maxAx, float maxAy,
		float minBx, float minBy, float maxBx, float maxBy)
	{
		return !(maxAx < minBx ||
				 maxBx < minAx ||
				 maxAy < minBy ||
				 maxBy < minAy);
	}

	template<class Fn>
	void RenderSystem2D::forEachVisible(const Scene& scene,
										const Camera2D& cam,
										float aspect,
										Fn&& fn) const
	{
		// camera view bounds in world space
		const float halfH = cam.size / cam.zoom;
		const float halfW = cam.size * aspect / cam.zoom;

		const float vx0 = cam.x - halfW;
		const float vx1 = cam.x + halfW;
		const float vy0 = cam.y - halfH;
		const float vy1 = cam.y + halfH;

		for (const auto& e : scene.entities) {
			if (!e.renderable.visible) continue;
			if (!e.renderable.mesh) continue;

			//entity  AABB ignore rotation for now
			const float hw = 0.5f * std::abs(e.transform.sx);
			const float hh = 0.5f * std::abs(e.transform.sy);

			const float ex0 = e.transform.x - hw;
			const float ex1 = e.transform.x + hw;
			const float ey0 = e.transform.y - hh;
			const float ey1 = e.transform.y + hh;

			if (!aabbIntersects(ex0, ey0, ex1, ey1, vx0, vy0, vx1, vy1))
				continue;

			RenderPacket2D pkt;
			pkt.visible = true;
			pkt.mesh = e.renderable.mesh;
			pkt.model = e.transform.matrix();
			pkt.material = e.renderable.material;
			pkt.layer = e.renderable.layer;

			fn(pkt);
		}
	}

	void RenderSystem2D::buildPackets(const Scene& scene,
									  const Camera2D& cam,
									  float aspect,
									  RenderFrame2D& out) const
	{
		out.clearPackets();
		out.packets.reserve(scene.entities.size() + 16);

		forEachVisible(scene, cam, aspect, [&](const RenderPacket2D& pkt) {
			out.packets.push_back(pkt);
		});
	}

	void RenderSystem2D::submitVisible(const Scene& scene,
									   Renderer& renderer,
									   const Camera2D& cam,
									   float aspect) const
	{
		forEachVisible(scene, cam, aspect, [&](const RenderPacket2D& pkt) {
			renderer.submit(pkt);
		});
	}


}