#pragma once
#include <string>
#include <unordered_map>
#include "renderer/material2d.h" // Vec4

namespace argon {

	struct AtlasSpriteRectPx {
		int x = 0, y = 0, w = 0, h = 0;
	};

	class TextureAtlas {
	public:
		TextureAtlas(int texW = 0, int texH = 0) :m_texW(texW), m_texH(texH) {};
		void setTextureSize(int texW, int texH) { m_texW = texW; m_texH = texH; }
		bool loadFromFile(const std::string& path);
		Vec4 getUVRect(const std::string& name) const;
		bool has(const std::string& name) const;

	private:
		int m_texW = 0;
		int m_texH = 0;
		std::unordered_map<std::string, AtlasSpriteRectPx> m_rects;
	};
}