#include "ShaderLibrary.h"
#include "GraphicsAPI.h"

namespace ignis
{
	void ShaderLibrary::Add(std::unique_ptr<Shader> shader)
	{
		m_shaders[shader->GetName()] = std::move(shader);
	}

	Shader& ShaderLibrary::Load(const std::string& filepath)
	{
		m_shaders[filepath] = std::move(Shader::CreateFromFile(filepath));
		return *m_shaders[filepath];
	}

	Shader& ShaderLibrary::Get(const std::string& name)
	{
		return *m_shaders.at(name);
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_shaders.find(name) != m_shaders.end();
	}

}