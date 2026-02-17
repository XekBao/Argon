#pragma once
#include "material_library.h"

namespace argon {

	MaterialHandle argon::MaterialLibrary::add(const Material2D& m) {
		m_materials.push_back(m);
		return (MaterialHandle)m_materials.size(); // 1-based, 0 = invalid
	}

	const Material2D* MaterialLibrary::get(MaterialHandle h) const {
		if (h == kInvalidMaterial) return nullptr;
		const std::size_t idx = (std::size_t)h - 1;
		if (idx >= m_materials.size()) return nullptr;
		return &m_materials[idx];
	}

	Material2D* MaterialLibrary::get(MaterialHandle h) {
		if (h == kInvalidMaterial) return nullptr;
		const std::size_t idx = (std::size_t)h - 1;
		if (idx >= m_materials.size()) return nullptr;
		return &m_materials[idx];
	}

}