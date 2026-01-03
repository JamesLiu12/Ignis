#pragma once

#include "Material.h"

namespace ignis
{
	class Environment;
	struct LightEnvironment;

	class Pipeline
	{	
	public:
		virtual ~Pipeline() = default;

		virtual std::shared_ptr<Material> CreateMaterial(const MaterialData& data) = 0;
		virtual void ApplyEnvironment(Material& material, const Environment& scene_environment, const LightEnvironment& light_environment) = 0;
		virtual std::shared_ptr<Shader> GetStandardShader() = 0;
	};
}