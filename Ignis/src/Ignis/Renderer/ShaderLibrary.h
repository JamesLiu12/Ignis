#pragma once

#include "Shader.h"

namespace ignis
{
	class ShaderLibrary
	{
	public:
		void Add(std::unique_ptr<Shader> shader);
		Shader& Load(const std::string& filepath);
		Shader& Get(const std::string& name);
		bool Exists(const std::string& name) const;

	public:
		std::unordered_map<std::string, std::unique_ptr<Shader>> m_shaders = {};
	};
}