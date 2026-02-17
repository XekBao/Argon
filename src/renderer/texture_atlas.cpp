#include "renderer/texture_atlas.h"
#include <fstream>
#include <sstream>

namespace argon {

	static inline Vec4 rectPxToUV(int texW, int texH, const AtlasSpriteRectPx& r) {
		if (texW <= 0 || texH <= 0) return { 0,0,1,1 };

		const float invW = 1.0f / (float)texW;
		const float invH = 1.0f / (float)texH;
		const float u0 = (r.x + 0.5f) * invW;
		const float v0 = (r.y + 0.5f) * invH;
		const float u1 = (r.x + r.w - 0.5f) * invW;
		const float v1 = (r.y + r.h - 0.5f) * invH;

		return { u0, v0, u1, v1 };
	}

	bool TextureAtlas::loadFromFile(const std::string& path) {
		std::ifstream in(path);
		if (!in.is_open()) return false;

		m_ids.clear();
		m_uvById.clear();

		m_uvById.push_back({ 0.0f, 0.0f, 1.0f, 1.0f });

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

			SpriteId id = 0;
			auto it = m_ids.find(name);
			if (it == m_ids.end()) {
				id = (SpriteId)m_uvById.size();
				m_ids[name] = id;
				m_uvById.push_back({ 0,0,1,1 });
			} else {
				id = it->second;
			}
			m_uvById[id] = rectPxToUV(m_texW, m_texH, r);
		}
		return true;
	}

	TextureAtlas::SpriteId TextureAtlas::getId(const std::string& name) const {
		auto it = m_ids.find(name);
		if (it == m_ids.end()) return 0;
		return it->second;
	}

	Vec4 TextureAtlas::uvRect(SpriteId id) const {
		if (id == 0) return { 0,0,1,1 };
		if (id >= m_uvById.size()) return { 0,0,1,1 };
		return m_uvById[id];
	}


}