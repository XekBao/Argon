#pragma once
#include <cstdint>
#include "math/mat4.h"
#include "renderer/material_handle.h"
#include "renderer/material2d.h"

namespace argon {

	class Mesh;

	struct RenderPacket2D {
		const Mesh* mesh = nullptr;
		MaterialHandle material = {};
		Mat4 model = Mat4::identity();
		std::int32_t layer = 0;
		bool visible = true;
		Vec4 tint{ 1.0f,1.0f,1.0f,1.0f };
		Vec4 uvRect{ 0.0f,0.0f,1.0f,1.0f }; // (u0,v0,u1,v1)
	};
}