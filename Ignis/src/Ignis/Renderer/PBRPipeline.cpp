#include "PBRPipeline.h"
#include "Ignis/Asset/AssetManager.h"
#include "Renderer.h"

namespace ignis
{
	static std::string PBRShaderPath = "assets://shaders/IgnisPBR.glsl";

	PBRPipeline::PBRPipeline(std::shared_ptr<ShaderLibrary> shader_library)
		: m_shader_library(std::move(shader_library))
	{
		if (!m_shader_library->Exists(PBRShaderPath))
		{
			m_shader_library->Load(PBRShaderPath);
		}
	}

	std::shared_ptr<Material> PBRPipeline::CreateMaterial(const MaterialData& data)
	{
		auto material = Material::Create(m_shader_library->Get(PBRShaderPath));
		
		material->GetShader()->Bind();

		material->Set("material.albedoMap", 0);
		material->Set("material.normalMap", 1);
		material->Set("material.metallicMap", 2);
		material->Set("material.roughnessMap", 3);
		material->Set("material.emissiveMap", 4);
		material->Set("material.aoMap", 5);

		if (auto texture = AssetManager::GetAsset<Texture2D>(data.AlbedoMap))
			texture->Bind(0);
		else
			Renderer::GetWhiteTexture()->Bind(0);

		if (auto texture = AssetManager::GetAsset<Texture2D>(data.NormalMap))
			texture->Bind(1);
		else
			Renderer::GetDefaultNormalTexture()->Bind(1);

		if (auto texture = AssetManager::GetAsset<Texture2D>(data.MetalnessMap))
			texture->Bind(2);
		else
			Renderer::GetBlackTexture()->Bind(2);

		if (auto texture = AssetManager::GetAsset<Texture2D>(data.RoughnessMap))
			texture->Bind(3);
		else
			Renderer::GetDefaultRoughnessTexture()->Bind(3);

		if (auto texture = AssetManager::GetAsset<Texture2D>(data.EmissiveMap))
			texture->Bind(4);
		else
			Renderer::GetBlackTexture()->Bind(4);

		if (auto texture = AssetManager::GetAsset<Texture2D>(data.AOMap))
			texture->Bind(5);
		else
			Renderer::GetWhiteTexture()->Bind(5);

		return material;
	}

	std::shared_ptr<Shader> PBRPipeline::GetStandardShader()
	{
		return m_shader_library->Get(PBRShaderPath);
	}
}