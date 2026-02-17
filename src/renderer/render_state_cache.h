#pragma once
#include <cstdint>

namespace argon {
	struct RenderStateCache {
		std::uint32_t shaderId = 0;
		std::uint32_t textureId = 0;
		std::uint32_t vaoId = 0;
	};
}