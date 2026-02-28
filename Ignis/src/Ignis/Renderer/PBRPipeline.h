#pragma once

#include "Pipeline.h"
#include "ShaderLibrary.h"

namespace ignis
{
	class PBRPipeline : public Pipeline
	{
	public:
		PBRPipeline(ShaderLibrary& shader_library);
		~PBRPipeline() = default;

		std::shared_ptr<Material> CreateMaterial(const MaterialData& data) override;
		void ApplyEnvironment(Material& material, const Environment& scene_environment, const EnvironmentSettings& environment_settings, const LightEnvironment& light_environment) override;
		std::shared_ptr<Material> CreateSkyboxMaterial(const Environment& scene_environment) override;
		std::shared_ptr<Shader> GetStandardShader() override;
		std::shared_ptr<Shader> GetSkyboxShader() override;

	private:
		ShaderLibrary& m_shader_library;
		std::shared_ptr<Texture2D> m_brdf_lut_texture;
	};
}