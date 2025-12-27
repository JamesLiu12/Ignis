#pragma once

#include "Pipeline.h"
#include "ShaderLibrary.h"

namespace ignis
{
	class PBRPipeline : public Pipeline
	{
	public:
		PBRPipeline(std::shared_ptr<ShaderLibrary> shader_library);
		~PBRPipeline() = default;

		std::shared_ptr<Material> CreateMaterial(const MaterialData& data) override;
		std::shared_ptr<Shader> GetStandardShader() override;

	private:
		std::shared_ptr<ShaderLibrary> m_shader_library;
	};
}