#pragma once

#include "Ignis/Renderer/Shader.h"

namespace ignis
{
	class GLShader : public Shader
	{
	public:
		GLShader(const std::string& name, const std::string& vertex_source, const std::string& fragment_source);
		GLShader(const std::string& filepath);

		~GLShader() override;

		void Bind();
		void UnBind();

		const std::string& GetName() const override;

		friend class GLMaterial;

	private:
		uint32_t m_id = 0;
		std::string m_name = "";
	};
}