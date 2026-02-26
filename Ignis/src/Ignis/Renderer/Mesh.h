#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Material.h"
#include "Ignis/Asset/Asset.h"

#include <glm/glm.hpp>

namespace ignis
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;
	};

	struct MeshNode
	{
		uint32_t ParentIndex = 0xffffffff;
		std::vector<uint32_t> ChildrenIndices;
		glm::mat4 Transform{ 1.0f };
	};

	struct Submesh
	{
		uint32_t BaseVertex = 0;
		uint32_t BaseIndex = 0;
		uint32_t VertexCount = 0;
		uint32_t IndexCount = 0;
		uint32_t MaterialIndex = 0;
	};

	class Mesh : public Asset
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		Mesh() = default;

		AssetType GetAssetType() const override { return AssetType::Mesh; }

		MeshNode& GetRootNode() { return m_nodes[0]; }

		const std::vector<Vertex>& GetVertices() const { return m_vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_indices; }
		const std::vector<MaterialData>& GetMaterialsData() const { return m_materials_data; }
		const std::vector<MeshNode>& GetNodes() const { return m_nodes; }
		const std::vector<Submesh>& GetSubmeshes() const { return m_submeshes; }

		std::shared_ptr<VertexArray> GetVertexArray() const { return m_vertex_array; }
		std::shared_ptr<VertexBuffer> GetVertexBuffer() const { return m_vertex_buffer; }
		std::shared_ptr<IndexBuffer> GetIndexBuffer() const { return m_index_buffer; }

		void SetMaterialDataTexture(uint32_t material_index, MaterialType type, AssetHandle texture_handle);
		void SetMaterialData(uint32_t material_index, MaterialData material_data);

		void FlipUVs();
		bool IsUVsFlipped() { return uv_flipped; }

		~Mesh() = default;

	private:
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;

		std::vector<MaterialData> m_materials_data;

		std::vector<MeshNode> m_nodes;
		std::vector<Submesh>  m_submeshes;

		std::shared_ptr<VertexArray> m_vertex_array;
		std::shared_ptr<VertexBuffer> m_vertex_buffer;
		std::shared_ptr<IndexBuffer> m_index_buffer;

		bool uv_flipped = false;

		friend class MeshImporter;
	};
}