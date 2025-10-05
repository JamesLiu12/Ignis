#pragma once

#include "Ignis/Renderer/IndexBuffer.h"

namespace ignis
{
	class GLIndexBuffer : public IndexBuffer
	{
	public:
		GLIndexBuffer(uint32_t* indices, uint32_t count);
		~GLIndexBuffer() override;

		void Bind() override;
		void Unbind() override;

		unsigned int GetCount() const override { return m_count; }

	private:
		uint32_t m_id;
		uint32_t m_count;
	};
}