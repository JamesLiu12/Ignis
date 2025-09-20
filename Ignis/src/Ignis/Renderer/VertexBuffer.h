#pragma once

namespace ignis
{
	class VertexBuffer
	{
	public:
		enum class Usage
		{
			Static,
			Dynamic
		};

		virtual ~VertexBuffer() = default;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual void SetData(float* data, size_t size) = 0;

		static std::shared_ptr<VertexBuffer> Create(size_t size, Usage usage = Usage::Dynamic);
		static std::shared_ptr<VertexBuffer> Create(float* vertices, size_t size, Usage usage = Usage::Static);
	};
}