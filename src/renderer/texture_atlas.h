#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "renderer/material2d.h" // Vec4

namespace argon {

	struct AtlasSpriteRectPx {
		int x = 0, y = 0, w = 0, h = 0;
	};

	class TextureAtlas {
	public:
		using SpriteId = std::uint32_t;

		TextureAtlas(int texW = 0, int texH = 0) :m_texW(texW), m_texH(texH) {};
		
		void setTextureSize(int texW, int texH) { m_texW = texW; m_texH = texH; }
		bool loadFromFile(const std::string& path);

		SpriteId getId(const std::string& name) const;
		Vec4 uvRect(SpriteId id) const;
		
	private:
		int m_texW = 0;
		int m_texH = 0;
		std::unordered_map<std::string, SpriteId> m_ids;
		std::vector<Vec4> m_uvById;
	};
}