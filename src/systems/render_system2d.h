#pragma once
#include <cstdint>

namespace argon {
	
	class Scene;
	class Renderer;
	class Camera2D;
	struct RenderFrame2D;
	struct RenderPacket2D;

	class RenderSystem2D {
	public:
		void submitVisible(const Scene& scene, Renderer& renderer,
						   const Camera2D& cam, float aspect) const;

		void buildPackets(const Scene& scene, Renderer& renderer, const Camera2D& cam,
						  float aspect, RenderFrame2D& out) const;

	private:
		template<class Fn>
		void forEachVisible(const Scene& scene, const Renderer& renderer,
			const Camera2D& cam, float aspect, Fn&& fn) const;
	};

}