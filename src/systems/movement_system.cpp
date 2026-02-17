#include "movement_system.h"
#include "scene/scene.h"
#include "platform/window.h"
#include "input/input_map.h"
#include <cmath>


namespace argon {
	void argon::MovementSystem::update(Scene& scene,
									   const Window& win,
									   const InputMap& input,
									   float dt) const {

		for (auto& e : scene.entities) {
			if (!e.controllable) continue;

			float dx = 0.0f, dy = 0.0f;
			if (input.down(win, Action::MoveLeft))	dx -= 1.0f;
			if (input.down(win, Action::MoveRight)) dx += 1.0f;
			if (input.down(win, Action::MoveUp))	dy += 1.0f;
			if (input.down(win, Action::MoveDown))	dy -= 1.0f;

			if (dx != 0.0f || dy != 0.0f) {
				float len = std::sqrt(dx * dx + dy * dy);
				dx /= len; dy /= len;

				e.transform.x += dx * speed * dt;
				e.transform.y += dy * speed * dt;
			}
		}
	}
}