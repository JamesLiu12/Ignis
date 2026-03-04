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

		const std::string& GetName() const override { return m_name; }

		const std::unordered_map<std::string, ShaderUniform>& GetUniforms() const override { return m_uniforms; }
		const std::unordered_map<std::string, ShaderSampler>& GetSamplers() const override { return m_samplers; }
		uint32_t GetUniformBufferSize() const override { return m_uniformBufferSize; }

		int32_t GetUniformLocation(const std::string& name) const;

	private:
		void Reflect();

		uint32_t m_id = 0;
		std::string m_name = "";

		std::unordered_map<std::string, ShaderUniform> m_uniforms;
		std::unordered_map<std::string, ShaderSampler> m_samplers;
		uint32_t m_uniformBufferSize = 0;

		mutable std::unordered_map<std::string, int32_t> m_locationCache;

		friend class GLMaterial;
	};
}