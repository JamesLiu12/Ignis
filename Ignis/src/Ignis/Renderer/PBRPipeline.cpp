#include "PBRPipeline.h"

namespace ignis
{
	static std::string PBRShaderPath = "assets://shaders/IgnisPBR.glsl";

	PBRPipeline::PBRPipeline(std::shared_ptr<ShaderLibrary> shader_library)
		: m_shader_library(std::move(shader_library))
	{
		if (!m_shader_library->Exists(PBRShaderPath))
		{
			m_shader_library->Load(PBRShaderPath);
		}
	}

	std::shared_ptr<Material> PBRPipeline::CreateMaterial(const MaterialData& data)
	{
		auto material = Material::Create(m_shader_library->Get(PBRShaderPath));

		// TODO Set uniforms

		return material;
	}

	std::shared_ptr<Shader> PBRPipeline::GetStandardShader()
	{
		return m_shader_library->Get(PBRShaderPath);
	}
}