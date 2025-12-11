#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "Material.h"

#include <glm/glm.hpp>

namespace ignis
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	};

	struct MeshNode
	{
		int32_t ParentIndex = -1;
		std::vector<int32_t> ChildrenIndices;
		glm::mat4 Transform{ 1.0f };
	};

	struct Submesh
	{
		uint32_t BaseVertex = 0;
		uint32_t BaseIndex = 0;
		uint32_t IndexCount = 0;
		uint32_t MaterialIndex = 0;
	};

	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		Mesh() = default;
		MeshNode& GetRootNode() { return m_nodes[0]; }

		~Mesh() = default;

	private:
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;

		std::vector<Material> m_materials;

		std::vector<MeshNode> m_nodes;
		std::vector<Submesh>  m_submeshes;

		std::shared_ptr<VertexBuffer> m_vertex_buffer;
		std::shared_ptr<IndexBuffer> m_index_buffer;

		friend class MeshImporter;
	};
}