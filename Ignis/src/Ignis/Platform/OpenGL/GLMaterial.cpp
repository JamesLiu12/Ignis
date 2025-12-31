#include "GLMaterial.h"
#include <glad/glad.h>

namespace ignis
{
	GLMaterial::GLMaterial(std::shared_ptr<Shader> shader, const std::string& name)
		: m_shader(std::dynamic_pointer_cast<GLShader>(std::move(shader))), m_name(name)
	{

	}

	GLMaterial::GLMaterial(std::shared_ptr<Material> other, const std::string& name)
		: m_shader(std::dynamic_pointer_cast<GLShader>(other->GetShader())), m_name(name)
	{

	}

	void GLMaterial::Set(const std::string& name, float value)
	{
		glUniform1f(glGetUniformLocation(m_shader->m_id, name.c_str()), value);
	}

	void GLMaterial::Set(const std::string& name, const glm::vec2& vector)
	{
		glUniform2fv(glGetUniformLocation(m_shader->m_id, name.c_str()), 1, &vector[0]);
	}

	void GLMaterial::Set(const std::string& name, const glm::vec3& vector)
	{
		glUniform3fv(glGetUniformLocation(m_shader->m_id, name.c_str()), 1, &vector[0]);
	}

	void GLMaterial::Set(const std::string& name, const glm::vec4& vector)
	{
		glUniform4fv(glGetUniformLocation(m_shader->m_id, name.c_str()), 1, &vector[0]);
	}

	void GLMaterial::Set(const std::string& name, int value)
	{
		glUniform1i(glGetUniformLocation(m_shader->m_id, name.c_str()), value);
	}

	void GLMaterial::Set(const std::string& name, const glm::ivec2& vector)
	{
		glUniform2iv(glGetUniformLocation(m_shader->m_id, name.c_str()), 1, &vector[0]);
	}

	void GLMaterial::Set(const std::string& name, const glm::ivec3& vector)
	{
		glUniform3iv(glGetUniformLocation(m_shader->m_id, name.c_str()), 1, &vector[0]);
	}

	void GLMaterial::Set(const std::string& name, const glm::ivec4& vector)
	{
		glUniform4iv(glGetUniformLocation(m_shader->m_id, name.c_str()), 1, &vector[0]);
	}

	void GLMaterial::Set(const std::string& name, unsigned int value)
	{
		glUniform1ui(glGetUniformLocation(m_shader->m_id, name.c_str()), value);
	}

	void GLMaterial::Set(const std::string& name, const glm::uvec2& vector)
	{
		glUniform2uiv(glGetUniformLocation(m_shader->m_id, name.c_str()), 1, &vector[0]);
	}

	void GLMaterial::Set(const std::string& name, const glm::uvec3& vector)
	{
		glUniform3uiv(glGetUniformLocation(m_shader->m_id, name.c_str()), 1, &vector[0]);
	}

	void GLMaterial::Set(const std::string& name, const glm::uvec4& vector)
	{
		glUniform4uiv(glGetUniformLocation(m_shader->m_id, name.c_str()), 1, &vector[0]);
	}

	void GLMaterial::Set(const std::string& name, const glm::mat3& matrix)
	{
		glUniformMatrix3fv(glGetUniformLocation(m_shader->m_id, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
	}

	void GLMaterial::Set(const std::string& name, const glm::mat4& matrix)
	{
		glUniformMatrix4fv(glGetUniformLocation(m_shader->m_id, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
	}
}