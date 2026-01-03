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
		void ApplyEnvironment(Material& material, const Environment& scene_environment, const EnvironmentSettings& environment_settings, const LightEnvironment& light_environment) override;
		std::shared_ptr<Shader> GetStandardShader() override;

	private:
		std::shared_ptr<ShaderLibrary> m_shader_library;
		std::shared_ptr<Texture2D> m_brdf_lut_texture;
	};
}