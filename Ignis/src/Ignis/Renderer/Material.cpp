#include "Material.h"

namespace ignis
{
	void Material::SetTexture(MaterialType type, const std::shared_ptr<Texture>& texture)
	{
		switch (type)
		{
		case MaterialType::Diffuse:
			m_maps.Diffuse = texture;
			break;
		case MaterialType::Specular:
			m_maps.Specular = texture;
			break;
		case MaterialType::Normal:
			m_maps.Normal = texture;
			break;
		}
	}
	std::shared_ptr<Texture> Material::GetTexture(MaterialType type) const
	{
		switch (type)
		{
		case MaterialType::Diffuse:
			return m_maps.Diffuse;
		case MaterialType::Specular:
			return m_maps.Specular;
		case MaterialType::Normal:
			return m_maps.Normal;
		default:
			return nullptr;
		}
	}
}