#include "MeshImporter.h"
#include "Ignis/Renderer/VertexBuffer.h"
#include "Ignis/Renderer/IndexBuffer.h"
#include "TextureImporter.h"
#include "AssetManager.h"

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

	uint32_t MeshImporter::BuildMeshNodeHierarchy(const void* ainode, uint32_t parent_index, Mesh& mesh)
	{
		const aiNode* actual_node = static_cast<const aiNode*>(ainode);
		MeshNode mesh_node;
		mesh_node.ParentIndex = parent_index;
		mesh_node.Transform = AIToGLMMat4(actual_node->mTransformation);

		uint32_t this_index = mesh.m_nodes.size();
		mesh.m_nodes.push_back(mesh_node);

		if (parent_index != 0xffffffff)
			mesh.m_nodes[parent_index].ChildrenIndices.push_back(this_index);

		for (uint32_t i = 0; i < actual_node->mNumChildren; ++i)
		{
			const aiNode* child = actual_node->mChildren[i];
			MeshImporter::BuildMeshNodeHierarchy(child, this_index, mesh);
		}

		return this_index;
	}

	static void LoadMaterialTextures(
		const aiMaterial* aimat,
		const std::string& model_dir,
		MaterialData& out_material_data
	)
	{
		auto loadTexture = [&](const aiString& rel_path) -> AssetHandle
		{
			std::string tex_path = VFS::ConcatPath(model_dir, rel_path.C_Str());

			// Normalize path separators: convert backslashes to forward slashes for cross-platform compatibility
			// Windows accepts both / and \, but macOS/Linux only accept /
			std::replace(tex_path.begin(), tex_path.end(), '\\', '/');

			if (!VFS::Exists(tex_path))
			{
				Log::Warn("Texture file does not exist: {}", tex_path);
				return AssetHandle::Invalid;
			}
			return AssetManager::ImportAsset(tex_path);
		};

		if (aimat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString texture_path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path))
			{
				AssetHandle texture_handle = loadTexture(texture_path);
				if (texture_handle.IsValid())
				{
					out_material_data.AlbedoMap = texture_handle;
				}
			}
		}

		if (aimat->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			aiString texture_path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_NORMALS, 0, &texture_path))
			{
				AssetHandle texture_handle = loadTexture(texture_path);
				if (texture_handle.IsValid())
				{
					out_material_data.NormalMap = texture_handle;
				}
			}
		}
		else if (aimat->GetTextureCount(aiTextureType_HEIGHT) > 0)
		{
			aiString texture_path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_HEIGHT, 0, &texture_path))
			{
				AssetHandle texture_handle = loadTexture(texture_path);
				if (texture_handle.IsValid())
				{
					out_material_data.NormalMap = texture_handle;
				}
			}
		}

		if (aimat->GetTextureCount(aiTextureType_METALNESS) > 0)
		{
			aiString texture_path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_METALNESS, 0, &texture_path))
			{
				AssetHandle texture_handle = loadTexture(texture_path);
				if (texture_handle.IsValid())
				{
					out_material_data.MetalnessMap = texture_handle;
				}
			}
		}

		if (aimat->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
		{
			aiString texture_path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texture_path))
			{
				AssetHandle texture_handle = loadTexture(texture_path);
				if (texture_handle.IsValid())
				{
					out_material_data.RoughnessMap = texture_handle;
				}
			}
		}

		if (aimat->GetTextureCount(aiTextureType_EMISSIVE) > 0)
		{
			aiString texture_path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_EMISSIVE, 0, &texture_path))
			{
				AssetHandle texture_handle = loadTexture(texture_path);
				if (texture_handle.IsValid())
				{
					out_material_data.EmissiveMap = texture_handle;
				}
			}
		}

		if (aimat->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0)
		{
			aiString texture_path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texture_path))
			{
				AssetHandle texture_handle = loadTexture(texture_path);
				if (texture_handle.IsValid())
				{
					out_material_data.AOMap = texture_handle;
				}
			}
		}
	}

	std::shared_ptr<Mesh> MeshImporter::ImportMesh(const std::string& filepath)
	{
		auto mesh = std::make_shared<Mesh>();

		Assimp::Importer importer;
		auto resolved = VFS::Resolve(filepath);
		std::filesystem::path model_path = resolved;

		const aiScene* scene = importer.ReadFile(
			model_path.string(),
			aiProcess_Triangulate
			| aiProcess_GenSmoothNormals
			| aiProcess_CalcTangentSpace

		);

		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		{
			Log::Error("Assimp Error while loading {}: {}", filepath, importer.GetErrorString());
			return nullptr;
		}

		mesh->m_nodes.reserve(scene->mNumMeshes);
		MeshImporter::BuildMeshNodeHierarchy(scene->mRootNode, 0xffffffff, *mesh);

		mesh->m_materials_data.clear();
		mesh->m_materials_data.resize(scene->mNumMaterials);

		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
		{
			aiMaterial* aimat = scene->mMaterials[i];
			LoadMaterialTextures(aimat, VFS::ParentPath(filepath), mesh->m_materials_data[i]);
		}

		mesh->m_vertices.clear();
		mesh->m_indices.clear();
		mesh->m_submeshes.clear();

		uint32_t base_vertex = 0;
		uint32_t base_index = 0;

		for (unsigned int mesh_index = 0; mesh_index < scene->mNumMeshes; ++mesh_index)
		{
			aiMesh* aimesh = scene->mMeshes[mesh_index];

			Submesh sub;
			sub.BaseVertex = base_vertex;
			sub.BaseIndex = base_index;
			sub.MaterialIndex = aimesh->mMaterialIndex;

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

				if (aimesh->HasTangentsAndBitangents())
				{
					vertex.Tangent = glm::vec3(aimesh->mTangents[v].x, aimesh->mTangents[v].y, aimesh->mTangents[v].z);
					vertex.Bitangent = glm::vec3(aimesh->mBitangents[v].x, aimesh->mBitangents[v].y, aimesh->mBitangents[v].z);
				}
				else
				{
					vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
					vertex.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
				}

				mesh->m_vertices.push_back(vertex);
			}			

			for (unsigned int f = 0; f < aimesh->mNumFaces; ++f)
			{
				const aiFace& face = aimesh->mFaces[f];
				for (unsigned int i = 0; i < face.mNumIndices; ++i)
				{
					mesh->m_indices.push_back(base_vertex + face.mIndices[i]);
				}
			}

			sub.VertexCount = static_cast<uint32_t>(mesh->m_vertices.size()) - sub.BaseVertex;
			sub.IndexCount = static_cast<uint32_t>(mesh->m_indices.size()) - sub.BaseIndex;
			mesh->m_submeshes.push_back(sub);

			base_vertex = static_cast<uint32_t>(mesh->m_vertices.size());
			base_index = static_cast<uint32_t>(mesh->m_indices.size());
		}

		mesh->m_vertex_array = VertexArray::Create();

		mesh->m_vertex_buffer = VertexBuffer::Create(mesh->m_vertices.data(),
			(uint32_t)(mesh->m_vertices.size() * sizeof(Vertex)));

		mesh->m_vertex_buffer->SetLayout(VertexBuffer::Layout({
			{0, Shader::DataType::Float3},
			{1, Shader::DataType::Float3},
			{2, Shader::DataType::Float2},
			{3, Shader::DataType::Float3},
			{4, Shader::DataType::Float3},
			}));

		mesh->m_index_buffer = IndexBuffer::Create(mesh->m_indices.data(),
			(uint32_t)(mesh->m_indices.size() * sizeof(uint32_t)));

		mesh->m_vertex_array->AddVertexBuffer(mesh->m_vertex_buffer);
		mesh->m_vertex_array->SetIndexBuffer(mesh->m_index_buffer);

		return mesh;
	}
}