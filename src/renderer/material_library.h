#pragma once
#include <vector>
#include "renderer/material2d.h"
#include "renderer/material_handle.h"  

namespace argon {

	class MaterialLibrary {
	public:
		MaterialHandle add(const Material2D& m);
		const Material2D* get(MaterialHandle h) const;
		Material2D* get(MaterialHandle h);

	private:
		std::vector<Material2D> m_materials;
	};
}