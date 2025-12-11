#include "Mesh.h"

namespace ignis
{
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		: m_vertices(vertices), m_indices(indices)
	{
		m_vertex_buffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(Vertex));
		m_index_buffer = IndexBuffer::Create(indices.data(), indices.size() * sizeof(uint32_t));
	}
}