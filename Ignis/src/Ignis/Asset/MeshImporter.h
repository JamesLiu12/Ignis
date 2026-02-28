#pragma once

#include "Ignis/Renderer/Mesh.h"
#include "AssetImporter.h"

namespace ignis
{
	class MeshImporter : public AssetImporter
	{
	public:
		AssetType GetType() const override;
		std::shared_ptr<Asset> Import(const std::string& path, const AssetLoadContext& context) override;

		static MeshImporter& Get();

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