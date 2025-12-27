#pragma once

#include "Material.h"

namespace ignis
{
	class Pipeline
	{	
	public:
		virtual ~Pipeline() = default;

		virtual std::shared_ptr<Material> CreateMaterial(const MaterialData& data) = 0;
		virtual std::shared_ptr<Shader> GetStandardShader() = 0;
	};
}