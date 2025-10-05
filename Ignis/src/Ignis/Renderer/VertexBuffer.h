#pragma once

#include "Shader.h"

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

		struct Attribute
		{
			uint32_t Index;
			Shader::DataType Type;
			bool Normalized;
			size_t Offset;

			Attribute(uint32_t index, Shader::DataType type, bool normalized = false, size_t offset = 0)
				: Index(index), Type(type), Normalized(normalized), Offset(offset) {
			}
		};

		class Layout
		{
		public:
			Layout(const std::initializer_list<Attribute>& attributes);
			~Layout() = default;

			const std::vector<Attribute>& GetAttributes() const { return m_attributes; }
			size_t GetStride() const { return m_stride; }

		private:
			std::vector<Attribute> m_attributes;
			size_t m_stride;
		};

		virtual ~VertexBuffer() = default;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual void SetData(float* data, size_t size) = 0;

		virtual const Layout& GetLayout() const { return m_layout; }
		virtual void SetLayout(const Layout& layout) { m_layout = layout; }

		static std::shared_ptr<VertexBuffer> Create(size_t size, Usage usage = Usage::Dynamic);
		static std::shared_ptr<VertexBuffer> Create(float* vertices, size_t size, Usage usage = Usage::Static);

	protected:
		Layout m_layout = {};
	};
}