#pragma once

namespace ignis
{
	class GraphicsAPI
	{
	public:
		enum class Type
		{
			OpenGL
		};

		static Type GetType()
		{
			static Type type = Type::OpenGL;
			return type;
		}
	};
}