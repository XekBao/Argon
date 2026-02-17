#pragma once
#include <unordered_map>
#include <GLFW/glfw3.h>
#include "input/actions.h"
#include "platform/window.h"

namespace argon {

	class InputMap {
	public:
		void bind(Action a, int key) { m_key[a] = key; }

		bool down(const Window& w, Action a) const {
			auto it = m_key.find(a);
			if (it == m_key.end()) return false;
			return w.keyDown(it->second);
		}

	private:
		std::unordered_map<Action, int> m_key;

	};


}