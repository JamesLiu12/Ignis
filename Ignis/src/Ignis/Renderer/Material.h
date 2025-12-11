#pragma once

#include "Texture.h"

namespace ignis
{
	enum class MaterialType
	{
		Diffuse,
		Specular,
		Normal,
	};

	class Material
	{
	public:
		Material() = default;
		~Material() = default;
		void SetTexture(MaterialType type, const std::shared_ptr<Texture>& texture);
		std::shared_ptr<Texture> GetTexture(MaterialType type) const;

	private:
		struct Maps
		{
			std::shared_ptr<Texture> Diffuse;
			std::shared_ptr<Texture> Specular;
			std::shared_ptr<Texture> Normal;
		} m_maps;
	};
}