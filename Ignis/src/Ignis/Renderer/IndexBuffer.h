#pragma once

namespace ignis
{
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual unsigned int GetCount() const = 0;

		static std::shared_ptr<IndexBuffer> Create(const uint32_t* indices, uint32_t size);
	};
}