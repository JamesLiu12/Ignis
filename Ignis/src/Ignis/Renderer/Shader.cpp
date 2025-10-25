#include "Shader.h"
#include "Ignis/Platform/OpenGL/GLShader.h"
#include "GraphicsAPI.h"

namespace ignis
{
	std::unique_ptr<Shader> Shader::Create(const std::string& name, const std::string & vertex_source, const std::string& fragment_source)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_unique<GLShader>(name, vertex_source, fragment_source);
		default:
			return nullptr;
		}
	}
	std::unique_ptr<Shader> Shader::CreateFromFile(const std::string& filepath)
	{
		switch (GraphicsAPI::GetType())
		{
		case GraphicsAPI::Type::OpenGL:
			return std::make_unique<GLShader>(filepath);
		default:
			return nullptr;
		}
	}
}