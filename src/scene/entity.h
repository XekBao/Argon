#pragma once

#include "math/transform.h"
#include "renderer/material2d.h"
#include "renderer/mesh.h"
#include "renderer/material_handle.h"


namespace argon {

	struct Renderable2D {
		const Mesh* mesh = nullptr;
		MaterialHandle material = kInvalidMaterial;
		bool visible = true;
		std::uint32_t layer = 0; //default world layer
	};


	struct Entity {
		Transform transform{};
		Renderable2D renderable{};
		bool controllable = false;
	};
}