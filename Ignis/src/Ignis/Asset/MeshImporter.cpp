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

	static UVTransform ReadUVTransform(const aiMaterial* aimat, aiTextureType type, unsigned int index)
	{
		UVTransform result;
		aiUVTransform aiT;
		if (AI_SUCCESS == aimat->Get(AI_MATKEY_UVTRANSFORM(type, index), aiT))
		{
			result.Offset = { aiT.mTranslation.x, aiT.mTranslation.y };
			result.Scale = { aiT.mScaling.x,     aiT.mScaling.y };
			result.Rotation = aiT.mRotation;
		}
		return result;
	}

	static void LoadMaterialTextures(
		const aiMaterial* aimat,
		const std::string& model_dir,
		MaterialData& out_material_data
	)
	{
		auto loadTextureSRGB = [&](const aiString& rel_path) -> AssetHandle
			{
				Log::Info("Loading sRGB texture: {}", rel_path.C_Str());
				std::string tex_path = VFS::ConcatPath(model_dir, rel_path.C_Str());
				std::replace(tex_path.begin(), tex_path.end(), '\\', '/');

				if (!VFS::Exists(tex_path))
				{
					Log::Warn("Texture file does not exist: {}", tex_path);
					return AssetHandle::Invalid;
				}

				TextureImportOptions opts;
				opts.InternalFormat = TextureFormat::RGBA8_sRGB;
				return AssetManager::ImportAsset(tex_path, AssetType::Texture2D, opts);
			};


		auto loadTextureLinear = [&](const aiString& rel_path) -> AssetHandle
			{
				Log::Info("Loading linear texture: {}", rel_path.C_Str());
				std::string tex_path = VFS::ConcatPath(model_dir, rel_path.C_Str());
				std::replace(tex_path.begin(), tex_path.end(), '\\', '/');

				if (!VFS::Exists(tex_path))
				{
					Log::Warn("Texture file does not exist: {}", tex_path);
					return AssetHandle::Invalid;
				}

				return AssetManager::ImportAsset(tex_path, AssetType::Texture2D);
			};

		auto loadTextureFullEx = [&](
			aiTextureType type, unsigned int index,
			AssetHandle& out_handle, uint32_t& outUVIndex, UVTransform& out_transform,
			bool is_sRGB,
			std::string* outRawPath = nullptr) -> bool
			{
				aiString texture_path;
				unsigned int uvindex = 0;
				if (AI_SUCCESS != aimat->GetTexture(type, index, &texture_path, nullptr, &uvindex))
					return false;

				if (outRawPath)
					*outRawPath = texture_path.C_Str();

				out_handle = is_sRGB ? loadTextureSRGB(texture_path) : loadTextureLinear(texture_path);
				outUVIndex = static_cast<uint32_t>(uvindex);
				out_transform = ReadUVTransform(aimat, type, index);
				return out_handle.IsValid();
			};

		// Albedo Map
		{
			aiString path;
			unsigned int uv = 0;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_BASE_COLOR, 0, &path, nullptr, &uv))
			{
				loadTextureFullEx(aiTextureType_BASE_COLOR, 0,
					out_material_data.AlbedoMap, out_material_data.AlbedoMapUVIndex,
					out_material_data.AlbedoMapUVTransform, true);
			}
			else if (AI_SUCCESS == aimat->GetTexture(aiTextureType_DIFFUSE, 0, &path, nullptr, &uv))
			{
				loadTextureFullEx(aiTextureType_DIFFUSE, 0,
					out_material_data.AlbedoMap, out_material_data.AlbedoMapUVIndex,
					out_material_data.AlbedoMapUVTransform, true);
			}
		}
		// Albedo Color
		{
			aiColor4D base_color(1, 1, 1, 1);
			if (AI_SUCCESS == aimat->Get(AI_MATKEY_BASE_COLOR, base_color))
				out_material_data.AlbedoColor = { base_color.r, base_color.g, base_color.b, base_color.a };
			else
			{
				aiColor4D diffuse(1, 1, 1, 1);
				if (AI_SUCCESS == aimat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse))
					out_material_data.AlbedoColor = { diffuse.r, diffuse.g, diffuse.b, diffuse.a };
			}
		}

		// Normal map
		{
			aiString path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_NORMALS, 0, &path))
			{
				loadTextureFullEx(aiTextureType_NORMALS, 0,
					out_material_data.NormalMap, out_material_data.NormalMapUVIndex,
					out_material_data.NormalMapUVTransform, false);
			}
			else if (AI_SUCCESS == aimat->GetTexture(aiTextureType_HEIGHT, 0, &path))
			{
				loadTextureFullEx(aiTextureType_HEIGHT, 0,
					out_material_data.NormalMap, out_material_data.NormalMapUVIndex,
					out_material_data.NormalMapUVTransform, false);
			}
		}

		// Metallic Map
		std::string metallic_tex_path, roughness_tex_path;
		{
			aiString path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_METALNESS, 0, &path))
			{
				loadTextureFullEx(aiTextureType_METALNESS, 0,
					out_material_data.MetalnessMap, out_material_data.MetalnessMapUVIndex,
					out_material_data.MetalnessMapUVTransform, false,
					&metallic_tex_path);
			}
		}
		// Metallic Value
		{
			float metallic_factor = out_material_data.MetalnessMap.IsValid() ? 1.0f : 0.0f;
			aimat->Get(AI_MATKEY_METALLIC_FACTOR, metallic_factor);
			out_material_data.MetallicValue = metallic_factor;
		}

		// Roughness Map
		{
			aiString path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path))
			{
				loadTextureFullEx(aiTextureType_DIFFUSE_ROUGHNESS, 0,
					out_material_data.RoughnessMap, out_material_data.RoughnessMapUVIndex,
					out_material_data.RoughnessMapUVTransform, false,
					&roughness_tex_path);
			}
		}
		// Roughness Value
		{
			float roughness_factor = out_material_data.RoughnessMap.IsValid() ? 1.0f : 0.5f;
			aimat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness_factor);
			out_material_data.RoughnessValue = roughness_factor;
		}

		// RoughnessMetallic Map
		if (!metallic_tex_path.empty() && !roughness_tex_path.empty()
			&& metallic_tex_path == roughness_tex_path)
		{
			out_material_data.MetallicChannel = 2; // B
			out_material_data.RoughnessChannel = 1; // G
			Log::Info("Detected packed metallicRoughness: metallic=B, roughness=G");
		}
		else
		{
			out_material_data.MetallicChannel = 0;
			out_material_data.RoughnessChannel = 0;
		}


		// Emissive Map
		{
			aiString path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_EMISSIVE, 0, &path))
			{
				loadTextureFullEx(aiTextureType_EMISSIVE, 0,
					out_material_data.EmissiveMap, out_material_data.EmissiveMapUVIndex,
					out_material_data.EmissiveMapUVTransform, true);
			}
		}
		// Emissive Color and Intensity
		{
			aiColor3D color(0, 0, 0);
			if (AI_SUCCESS == aimat->Get(AI_MATKEY_COLOR_EMISSIVE, color))
				out_material_data.EmissiveColor = { color.r, color.g, color.b };
			else if (out_material_data.EmissiveMap.IsValid())
				out_material_data.EmissiveColor = glm::vec3(1.0f);

			float emissive_intensity = 1.0f;
			aimat->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissive_intensity);
			out_material_data.EmissiveIntensity = emissive_intensity;
		}

		// AO Map
		{
			aiString path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &path))
			{
				loadTextureFullEx(aiTextureType_AMBIENT_OCCLUSION, 0,
					out_material_data.AOMap, out_material_data.AOMapUVIndex,
					out_material_data.AOMapUVTransform, false);
			}
			else if (AI_SUCCESS == aimat->GetTexture(aiTextureType_LIGHTMAP, 0, &path))
			{
				loadTextureFullEx(aiTextureType_LIGHTMAP, 0,
					out_material_data.AOMap, out_material_data.AOMapUVIndex,
					out_material_data.AOMapUVTransform, false);
			}
		}

		// Clearcoat Factor
		{
			float clearcoat_factor = 0.0f;
			aimat->Get(AI_MATKEY_CLEARCOAT_FACTOR, clearcoat_factor);
			out_material_data.ClearcoatFactor = clearcoat_factor;
		}
		// Clearcoat Roughness Factor
		{
			float clear_coat_roughness_factor = 0.0f;
			aimat->Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, clear_coat_roughness_factor);
			out_material_data.ClearcoatRoughnessFactor = clear_coat_roughness_factor;
		}
		// Clearcoat Map (index 0)
		{
			aiString path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_CLEARCOAT, 0, &path))
			{
				loadTextureFullEx(aiTextureType_CLEARCOAT, 0,
					out_material_data.ClearcoatMap, out_material_data.ClearcoatMapUVIndex,
					out_material_data.ClearcoatMapUVTransform, false);
			}
		}
		// Clearcoat Roughness Map (index 1)
		{
			aiString path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_CLEARCOAT, 1, &path))
			{
				loadTextureFullEx(aiTextureType_CLEARCOAT, 1,
					out_material_data.ClearcoatRoughnessMap, out_material_data.ClearcoatRoughnessMapUVIndex,
					out_material_data.ClearcoatRoughnessMapUVTransform, false);
			}
		}
		// Clearcoat Normal Map (NORMALS index 1 or CLEARCOAT index 2)
		{
			aiString path;
			if (AI_SUCCESS == aimat->GetTexture(aiTextureType_NORMALS, 1, &path))
			{
				loadTextureFullEx(aiTextureType_NORMALS, 1,
					out_material_data.ClearcoatNormalMap, out_material_data.ClearcoatNormalMapUVIndex,
					out_material_data.ClearcoatNormalMapUVTransform, false);
			}
			else if (AI_SUCCESS == aimat->GetTexture(aiTextureType_CLEARCOAT, 2, &path))
			{
				loadTextureFullEx(aiTextureType_CLEARCOAT, 2,
					out_material_data.ClearcoatNormalMap, out_material_data.ClearcoatNormalMapUVIndex,
					out_material_data.ClearcoatNormalMapUVTransform, false);
			}
		}

		// Alpha Mode
		{
			aiString alpha_mode_str;
			if (AI_SUCCESS == aimat->Get("$mat.gltf.alphaMode", 0, 0, alpha_mode_str))
			{
				std::string mode = alpha_mode_str.C_Str();
				if (mode == "BLEND")
					out_material_data.Alpha = AlphaMode::Blend;
				else if (mode == "MASK")
					out_material_data.Alpha = AlphaMode::Mask;
				else
					out_material_data.Alpha = AlphaMode::Opaque;
			}
			else
			{
				float opacity = 1.0f;
				aimat->Get(AI_MATKEY_OPACITY, opacity);
				if (opacity < 1.0f)
					out_material_data.Alpha = AlphaMode::Blend;
			}

			float cutoff = 0.5f;
			aimat->Get("$mat.gltf.alphaCutoff", 0, 0, cutoff);
			out_material_data.AlphaCutoff = cutoff;
		}

		// Double-Sided
		{
			int two_sided = 0;
			aimat->Get(AI_MATKEY_TWOSIDED, two_sided);
			out_material_data.DoubleSided = (two_sided != 0);
		}
	}

	AssetType MeshImporter::GetType() const
	{
		return AssetType::Mesh;
	}

	std::shared_ptr<Asset> MeshImporter::Import(const AssetMetadata& metadata, const AssetLoadContext& context)
	{
		auto mesh = std::make_shared<Mesh>();

		Assimp::Importer importer;
		auto resolved = VFS::Resolve(metadata.FilePath);
		std::filesystem::path model_path = resolved;

		std::string ext = model_path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		uint32_t flags = aiProcess_Triangulate
			| aiProcess_GenSmoothNormals
			| aiProcess_CalcTangentSpace;

		if (ext == ".gltf" || ext == ".glb")
		{
			flags |= aiProcess_PreTransformVertices;
		}

		const aiScene* scene = importer.ReadFile(model_path.string(), flags);

		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		{
			Log::Error("Assimp Error while loading {}: {}", metadata.FilePath, importer.GetErrorString());
			return nullptr;
		}

		mesh->m_nodes.reserve(scene->mNumMeshes);
		MeshImporter::BuildMeshNodeHierarchy(scene->mRootNode, 0xffffffff, *mesh);

		mesh->m_materials_data.clear();
		mesh->m_materials_data.resize(scene->mNumMaterials);

		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
		{
			aiMaterial* aimat = scene->mMaterials[i];
			LoadMaterialTextures(aimat, VFS::ParentPath(metadata.FilePath), mesh->m_materials_data[i]);
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

				// Position
				if (aimesh->HasPositions())
				{
					vertex.Position.x = aimesh->mVertices[v].x;
					vertex.Position.y = aimesh->mVertices[v].y;
					vertex.Position.z = aimesh->mVertices[v].z;
				}

				// Normal
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

				// UV0
				if (aimesh->HasTextureCoords(0))
				{
					vertex.TexCoords.x = aimesh->mTextureCoords[0][v].x;
					vertex.TexCoords.y = aimesh->mTextureCoords[0][v].y;
				}
				else
				{
					vertex.TexCoords = glm::vec2(0.0f);
				}

				// UV1
				if (aimesh->HasTextureCoords(1))
				{
					vertex.TexCoords1.x = aimesh->mTextureCoords[1][v].x;
					vertex.TexCoords1.y = aimesh->mTextureCoords[1][v].y;
				}
				else
				{
					vertex.TexCoords1 = glm::vec2(0.0f);
				}

				// UV2
				if (aimesh->HasTextureCoords(2))
				{
					vertex.TexCoords2.x = aimesh->mTextureCoords[2][v].x;
					vertex.TexCoords2.y = aimesh->mTextureCoords[2][v].y;
				}
				else
				{
					vertex.TexCoords2 = glm::vec2(0.0f);
				}

				// Tangent & Bitangent
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
			{0, Shader::DataType::Float3},  // Position
			{1, Shader::DataType::Float3},  // Normal
			{2, Shader::DataType::Float2},  // TexCoords  (UV0)
			{3, Shader::DataType::Float2},  // TexCoords1 (UV1)
			{4, Shader::DataType::Float2},  // TexCoords2 (UV2)
			{5, Shader::DataType::Float3},  // Tangent
			{6, Shader::DataType::Float3},  // Bitangent
			}));

		mesh->m_index_buffer = IndexBuffer::Create(mesh->m_indices.data(),
			(uint32_t)(mesh->m_indices.size() * sizeof(uint32_t)));

		mesh->m_vertex_array->AddVertexBuffer(mesh->m_vertex_buffer);
		mesh->m_vertex_array->SetIndexBuffer(mesh->m_index_buffer);

		return mesh;
	}

	MeshImporter& MeshImporter::Get()
	{
		static MeshImporter instance;
		return instance;
	}
}