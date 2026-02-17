#pragma once
#include "renderer/shader.h"
#include "renderer/texture2d.h"

namespace argon {
	struct Vec4 {
		float r, g, b, a;
	};

	struct Material2D {
		const Shader* shader = nullptr;
		const Texture2D* texture = nullptr;
		Vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		bool useTexture = false;
	};
}