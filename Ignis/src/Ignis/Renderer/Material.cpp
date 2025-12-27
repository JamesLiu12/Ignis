#include "Material.h"
#include "Ignis/Platform/OpenGL/GLMaterial.h"

namespace ignis
{
	std::shared_ptr<Material> Material::Create(std::shared_ptr<Shader> shader, const std::string& name)
	{
		return std::make_shared<GLMaterial>(shader, name);
	}

	std::shared_ptr<Material> Material::Create(std::shared_ptr<Material> other, const std::string& name)
	{
		return std::make_shared<GLMaterial>(other, name);
	}
}