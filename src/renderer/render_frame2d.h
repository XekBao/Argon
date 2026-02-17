#pragma once
#include <vector>
#include "math/mat4.h"
#include "renderer/render_packet2d.h"

namespace argon {

	class Scene;
	class Camera2D;
	class MaterialLibrary;
	
	// when render time <= 1, direct draw
	enum class FrameMode {Auto = 0, Direct, Record};

	struct RenderFrame2D {
		
		FrameMode mode = FrameMode::Direct;

		Mat4 PV = Mat4::identity();
		const MaterialLibrary* matlib = nullptr;

		// direct path inputs
		const Scene* scene = nullptr;
		const Camera2D* cam = nullptr;
		float aspect = 1.0f;

		std::vector<RenderPacket2D> packets;
		void clearPackets() { packets.clear(); }
	};

}