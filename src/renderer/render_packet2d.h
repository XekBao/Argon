#pragma once
#include <cstdint>
#include "math/mat4.h"
#include "renderer/material_handle.h"

namespace argon {

	class Mesh;

	struct RenderPacket2D {
		const Mesh* mesh = nullptr;
		MaterialHandle material = {};
		Mat4 model = Mat4::identity();
		std::int32_t layer = 0;
		bool visible = true;
	};
}