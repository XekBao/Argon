#pragma once
#include <string>
#include <cstdint>
#include "math/transform.h"
#include "renderer/material2d.h"
#include "renderer/mesh.h"
#include "renderer/material_handle.h"
#include "scene/animation2d.h"


namespace argon {

	struct Renderable2D {
		const Mesh* mesh = nullptr;
		MaterialHandle material = kInvalidMaterial;
		bool visible = true;
		std::uint32_t layer = 0; //default world layer
		Vec4 tint{ 1.0f,1.0f,1.0f,1.0f };
		Vec4 uvRect{ 0.0f,0.0f,1.0f,1.0f }; // (u0,v0,u1,v1)
		std::uint32_t spriteId = 0;
	};


	struct Entity {
		Animator2D animator;
		bool animated = false;

		Transform transform{};
		Renderable2D renderable{};
		bool controllable = false;
	};
}