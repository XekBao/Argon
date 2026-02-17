#pragma once
#include <cmath>

namespace argon {

	class Scene;
	class Window;
	class InputMap;

	class MovementSystem {
	public:
		float speed = 2.5f;
		void update(Scene& scene,
					const Window& win,
					const InputMap& input,
					float dt) const;
	};
}