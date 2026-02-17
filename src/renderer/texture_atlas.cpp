#include "renderer/texture_atlas.h"
#include <fstream>
#include <sstream>

namespace argon {
	bool TextureAtlas::loadFromFile(const std::string& path) {
		std::ifstream in(path);
		if (!in.is_open()) return false;
		m_rects.clear();

		std::string line;
		while (std::getline(in, line)) {
			if (line.empty()) continue;
			if (line[0] == '#') continue;

			std::istringstream iss(line);
			std::string name;
			AtlasSpriteRectPx r;

			if (!(iss >> name >> r.x >> r.y >> r.w >> r.h)) {
				continue;
			}

			if (r.w <= 0 || r.h <= 0) continue;
			m_rects[name] = r;
		}
		return true;
	}

	Vec4 TextureAtlas::getUVRect(const std::string& name) const {
		if (m_texW <= 0 || m_texH <= 0) {
			return { 0.0f, 0.0f, 0.0f, 0.0f };
		}

		auto it = m_rects.find(name);
		if (it == m_rects.end()) {
			return { 0.0f, 0.0f, 0.0f, 0.0f };
		}

		const auto& r = it->second;

		// prevent bleeding
		const float invW = 1.0f / (float)m_texW;
		const float invH = 1.0f / (float)m_texH;

		const float u0 = (r.x + 0.5f) * invW;
		const float v0 = (r.y + 0.5f) * invH;
		const float u1 = (r.x + r.w - 0.5f) * invW;
		const float v1 = (r.y + r.h - 0.5f) * invH;

		return { u0, v0, u1, v1 };
	}

	bool TextureAtlas::has(const std::string& name) const {
		return m_rects.find(name) != m_rects.end();
	}

}