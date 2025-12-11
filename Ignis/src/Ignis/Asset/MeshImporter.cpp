#include "MeshImporter.h"
#include "Ignis/Renderer/VertexBuffer.h"
#include "Ignis/Renderer/IndexBuffer.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace ignis
{
	static glm::mat4 AIToGLMMat4(const aiMatrix4x4& m)
	{
		glm::mat4 result;
		result[0][0] = m.a1; result[1][0] = m.a2; result[2][0] = m.a3; result[3][0] = m.a4;
		result[0][1] = m.b1; result[1][1] = m.b2; result[2][1] = m.b3; result[3][1] = m.b4;
		result[0][2] = m.c1; result[1][2] = m.c2; result[2][2] = m.c3; result[3][2] = m.c4;
		result[0][3] = m.d1; result[1][3] = m.d2; result[2][3] = m.d3; result[3][3] = m.d4;
		return result;
	}

	int MeshImporter::BuildMeshNodeHierarchy(const void* ainode, int parentIndex, Mesh& mesh)
	{
		const aiNode* actualNode = static_cast<const aiNode*>(ainode);
		MeshNode node;
		node.ParentIndex = parentIndex;
		node.Transform = AIToGLMMat4(actualNode->mTransformation);

		int thisIndex = static_cast<int>(mesh.m_nodes.size());
		mesh.m_nodes.push_back(node);

		if (parentIndex >= 0)
			mesh.m_nodes[parentIndex].ChildrenIndices.push_back(thisIndex);

		for (unsigned int i = 0; i < actualNode->mNumChildren; ++i)
		{
			const aiNode* child = actualNode->mChildren[i];
			MeshImporter::BuildMeshNodeHierarchy(child, thisIndex, mesh);
		}

		return thisIndex;
	}
	// 仍然复用之前的 LoadMaterialTextures 辅助函数
	static void LoadMaterialTextures(
		const aiMaterial* aimat,
		const std::filesystem::path& modelDir,
		Material& outMaterial
	)
	{
		TextureSpecs specs;
		bool flipVertical = true;

		auto loadTexture = [&](const aiString& relPath) -> std::shared_ptr<Texture2D>
			{
				std::filesystem::path texPath = modelDir / relPath.C_Str();
				texPath = texPath.lexically_normal();

				if (!std::filesystem::exists(texPath))
				{
					Log::Warn("Texture file does not exist: {}", texPath.string());
					return nullptr;
				}

				return Texture2D::CreateFromFile(specs, texPath.string(), flipVertical);
			};

		// Diffuse
		if (aimat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString texPath;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
			{
				if (auto tex = loadTexture(texPath))
					outMaterial.SetTexture(MaterialType::Diffuse, tex);
			}
		}

		// Specular
		if (aimat->GetTextureCount(aiTextureType_SPECULAR) > 0)
		{
			aiString texPath;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_SPECULAR, 0, &texPath))
			{
				if (auto tex = loadTexture(texPath))
					outMaterial.SetTexture(MaterialType::Specular, tex);
			}
		}

		// Normal / Height -> Normal
		if (aimat->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			aiString texPath;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_NORMALS, 0, &texPath))
			{
				if (auto tex = loadTexture(texPath))
					outMaterial.SetTexture(MaterialType::Normal, tex);
			}
		}
		else if (aimat->GetTextureCount(aiTextureType_HEIGHT) > 0)
		{
			aiString texPath;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_HEIGHT, 0, &texPath))
			{
				if (auto tex = loadTexture(texPath))
					outMaterial.SetTexture(MaterialType::Normal, tex);
			}
		}
	}

	std::shared_ptr<Mesh> MeshImporter::ImportMesh(const std::string& filepath)
	{
		auto mesh = std::make_shared<Mesh>();

		Assimp::Importer importer;
		auto resolved = VFS::Resolve(filepath);
		std::filesystem::path modelPath = resolved;
		std::filesystem::path modelDir = modelPath.parent_path();

		const aiScene* scene = importer.ReadFile(
			modelPath.string(),
			aiProcess_Triangulate
			| aiProcess_GenSmoothNormals
			| aiProcess_FlipUVs
			| aiProcess_JoinIdenticalVertices
		);

		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		{
			Log::Error("Assimp Error while loading {}: {}", filepath, importer.GetErrorString());
			return nullptr;
		}

		// 1. 节点层级（略：跟之前一样）
		mesh->m_nodes.reserve(64);
		MeshImporter::BuildMeshNodeHierarchy(scene->mRootNode, -1, *mesh);

		// 2. 材质数组：一个 aiMaterial 对应一个 Material
		mesh->m_materials.clear();
		mesh->m_materials.resize(scene->mNumMaterials);

		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
		{
			aiMaterial* aimat = scene->mMaterials[i];
			LoadMaterialTextures(aimat, modelDir, mesh->m_materials[i]);
		}

		// 3. 顶点 + 索引 + Submesh（注意记录 MaterialIndex）
		mesh->m_vertices.clear();
		mesh->m_indices.clear();
		mesh->m_submeshes.clear();

		uint32_t baseVertex = 0;
		uint32_t baseIndex = 0;

		for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			aiMesh* aimesh = scene->mMeshes[meshIndex];

			Submesh sub;
			sub.BaseVertex = baseVertex;
			sub.BaseIndex = baseIndex;
			sub.MaterialIndex = aimesh->mMaterialIndex; // 直接映射到 m_materials 的下标

			// 顶点
			for (unsigned int v = 0; v < aimesh->mNumVertices; ++v)
			{
				Vertex vertex{};

				if (aimesh->HasPositions())
				{
					vertex.Position.x = aimesh->mVertices[v].x;
					vertex.Position.y = aimesh->mVertices[v].y;
					vertex.Position.z = aimesh->mVertices[v].z;
				}

				if (aimesh->HasNormals())
				{
					vertex.Normal.x = aimesh->mNormals[v].x;
					vertex.Normal.y = aimesh->mNormals[v].y;
					vertex.Normal.z = aimesh->mNormals[v].z;
				}
				else
				{
					vertex.Normal = glm::vec3(0, 1, 0);
				}

				if (aimesh->HasTextureCoords(0))
				{
					vertex.TexCoords.x = aimesh->mTextureCoords[0][v].x;
					vertex.TexCoords.y = aimesh->mTextureCoords[0][v].y;
				}
				else
				{
					vertex.TexCoords = glm::vec2(0.0f);
				}

				mesh->m_vertices.push_back(vertex);
			}

			// 索引
			for (unsigned int f = 0; f < aimesh->mNumFaces; ++f)
			{
				const aiFace& face = aimesh->mFaces[f];
				for (unsigned int i = 0; i < face.mNumIndices; ++i)
					mesh->m_indices.push_back(baseVertex + face.mIndices[i]);
			}

			sub.IndexCount = static_cast<uint32_t>(mesh->m_indices.size()) - sub.BaseIndex;
			mesh->m_submeshes.push_back(sub);

			baseVertex = static_cast<uint32_t>(mesh->m_vertices.size());
			baseIndex = static_cast<uint32_t>(mesh->m_indices.size());
		}

		mesh->m_vertex_buffer = VertexBuffer::Create(
			mesh->m_vertices.data(),
			mesh->m_vertices.size() * sizeof(Vertex)
		);

		mesh->m_index_buffer = IndexBuffer::Create(
			mesh->m_indices.data(),
			mesh->m_indices.size() * sizeof(uint32_t)
		);

		return mesh;
	}
}