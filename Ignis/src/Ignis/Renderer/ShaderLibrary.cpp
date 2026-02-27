#include "ShaderLibrary.h"
#include "GraphicsAPI.h"

namespace ignis
{
	void ShaderLibrary::Add(std::shared_ptr<Shader> shader)
	{
		m_shaders[shader->GetName()] = std::move(shader);
	}

	std::shared_ptr<Shader> ShaderLibrary::Load(const std::string& filepath, std::string_view name)
	{
		std::string key = name.empty() ? filepath : std::string(name);

		m_shaders[key] = std::move(Shader::CreateFromFile(filepath));
		return m_shaders[key];
	}

	std::shared_ptr<Shader> ShaderLibrary::Get(const std::string& name) const
	{
		return m_shaders.at(name);
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_shaders.find(name) != m_shaders.end();
	}

}