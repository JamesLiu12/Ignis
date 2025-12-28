#include "Mesh.h"

namespace ignis
{
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		: m_vertices(vertices), m_indices(indices)
	{
		m_vertex_buffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(Vertex));
		m_index_buffer = IndexBuffer::Create(indices.data(), indices.size() * sizeof(uint32_t));

		m_vertex_array = VertexArray::Create();

		m_vertex_buffer = VertexBuffer::Create(m_vertices.data(),
			(uint32_t)(m_vertices.size() * sizeof(Vertex)));

		m_vertex_buffer->SetLayout(VertexBuffer::Layout({
			{0, Shader::DataType::Float3, false, (uint32_t)offsetof(Vertex, Position)},
			{1, Shader::DataType::Float3, false, (uint32_t)offsetof(Vertex, Normal)},
			{2, Shader::DataType::Float2, false, (uint32_t)offsetof(Vertex, TexCoords)},
			}));

		m_index_buffer = IndexBuffer::Create(m_indices.data(),
			(uint32_t)(m_indices.size() * sizeof(uint32_t)));

		m_vertex_array->AddVertexBuffer(m_vertex_buffer);
		m_vertex_array->SetIndexBuffer(m_index_buffer);
	}

	void Mesh::SetMaterialDataTexture(uint32_t material_index, MaterialType type, AssetHandle texture_handle)
	{
		if (material_index >= m_materials_data.size()) return;

		MaterialData& data = m_materials_data[material_index];

		switch (type)
		{
		case MaterialType::Albedo:    data.AlbedoMap = texture_handle; break;
		case MaterialType::Normal:    data.NormalMap = texture_handle; break;
		case MaterialType::Metal:     data.MetalnessMap = texture_handle; break;
		case MaterialType::Roughness: data.RoughnessMap = texture_handle; break;
		case MaterialType::Emissive:  data.EmissiveMap = texture_handle; break;
		case MaterialType::AO:        data.AOMap = texture_handle; break;
		}
	}

	void Mesh::FlipUVs()
	{
		for (auto& vertex : m_vertices)
		{
			vertex.TexCoords.y = 1.0f - vertex.TexCoords.y;
		}

		if (m_vertex_buffer)
		{
			m_vertex_buffer->SetData(m_vertices.data(), m_vertices.size() * sizeof(Vertex));
		}
	}
}