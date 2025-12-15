#pragma once

#include "Ignis/Renderer/Mesh.h"

namespace ignis
{
	class MeshImporter
	{
	public:
		static std::shared_ptr<Mesh> ImportMesh(const std::string& filepath);

		MeshImporter(const MeshImporter&) = delete;
		MeshImporter& operator=(const MeshImporter&) = delete;
		MeshImporter(MeshImporter&&) = delete;
		MeshImporter& operator=(MeshImporter&&) = delete;

	private:
		MeshImporter() = default;
		~MeshImporter() = default;

		static uint32_t BuildMeshNodeHierarchy(const void* ainode, uint32_t parent_index, Mesh& mesh);
	};
}