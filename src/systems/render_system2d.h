#pragma once

namespace argon {
	
	class Scene;
	class Renderer;
	class Camera2D;
	class RenderFrame2D;

	class RenderSystem2D {
	public:
		void submitVisible(const Scene& scene, Renderer& renderer,
						   const Camera2D& cam, float aspect) const;

		void buildPackets(const Scene& scene, const Camera2D& cam,
						  float aspect, RenderFrame2D& out) const;
	};

}