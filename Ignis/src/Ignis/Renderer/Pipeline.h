#pragma once

#include "Material.h"

namespace ignis
{
	class Environment;
	struct EnvironmentSettings;
	struct LightEnvironment;

	class Pipeline
	{	
	public:
		virtual ~Pipeline() = default;

		virtual std::shared_ptr<Material> CreateMaterial(const MaterialData& data) = 0;
		virtual void ApplyEnvironment(Material& material, const Environment& scene_environment, const EnvironmentSettings& environment_settings, const LightEnvironment& light_environment) = 0;
		virtual std::shared_ptr<Material> CreateSkyboxMaterial(const Environment& scene_environment) = 0;
		virtual std::shared_ptr<Shader> GetStandardShader() = 0;
		virtual std::shared_ptr<Shader> GetSkyboxShader() = 0;
	};
}