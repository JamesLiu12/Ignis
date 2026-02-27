#pragma once

#include "Shader.h"

namespace ignis
{
	class ShaderLibrary
	{
	public:
		void Add(std::shared_ptr<Shader> shader);
		std::shared_ptr<Shader> Load(const std::string& filepath, std::string_view name = "");
		std::shared_ptr<Shader> Get(const std::string& name) const;
		bool Exists(const std::string& name) const;

	public:
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders = {};
	};
}