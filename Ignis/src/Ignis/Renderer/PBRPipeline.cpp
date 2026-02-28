#include "PBRPipeline.h"
#include "Ignis/Asset/AssetManager.h"
#include "Renderer.h"
#include "Environment.h"
#include "Ignis/Scene/Scene.h"

namespace ignis
{
	PBRPipeline::PBRPipeline(ShaderLibrary& shader_library)
		: m_shader_library(shader_library)
	{
		AssetHandle texture_handle = AssetManager::ImportAsset("resources://images/ibl_brdf_lut.png");
		m_brdf_lut_texture = AssetManager::GetAsset<Texture2D>(texture_handle);
	}

	std::shared_ptr<Material> PBRPipeline::CreateMaterial(const MaterialData& data)
	{
		auto material = Material::Create(m_shader_library.Get("IgnisPBR"));
		
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

	void PBRPipeline::ApplyEnvironment(Material& material, const Environment& scene_environment, const EnvironmentSettings& environment_settings, const LightEnvironment& light_environment)
	{
		material.Set("numDirectionalLights", (int)light_environment.DirectionalLights.size());
		for (size_t i = 0; i < light_environment.DirectionalLights.size(); i++)
		{
			std::string base = "directionalLights[" + std::to_string(i) + "]";
			material.Set(base + ".direction", light_environment.DirectionalLights[i].Direction);
			material.Set(base + ".radiance", light_environment.DirectionalLights[i].Radiance);
		}

		material.Set("numPointLights", (int)light_environment.PointLights.size());
		for (size_t i = 0; i < light_environment.PointLights.size(); i++)
		{
			std::string base = "pointLights[" + std::to_string(i) + "]";
			material.Set(base + ".position", light_environment.PointLights[i].Position);
			material.Set(base + ".radiance", light_environment.PointLights[i].Radiance);
			material.Set(base + ".constant", light_environment.PointLights[i].Constant);
			material.Set(base + ".linear", light_environment.PointLights[i].Linear);
			material.Set(base + ".quadratic", light_environment.PointLights[i].Quadratic);
		}

		material.Set("numSpotLights", (int)light_environment.SpotLights.size());
		for (size_t i = 0; i < light_environment.SpotLights.size(); i++)
		{
			std::string base = "spotLights[" + std::to_string(i) + "]";
			material.Set(base + ".position", light_environment.SpotLights[i].Position);
			material.Set(base + ".direction", light_environment.SpotLights[i].Direction);
			material.Set(base + ".radiance", light_environment.SpotLights[i].Radiance);
			material.Set(base + ".constant", light_environment.SpotLights[i].Constant);
			material.Set(base + ".linear", light_environment.SpotLights[i].Linear);
			material.Set(base + ".quadratic", light_environment.SpotLights[i].Quadratic);
			material.Set(base + ".cutOff", light_environment.SpotLights[i].CutOff);
			material.Set(base + ".outerCutOff", light_environment.SpotLights[i].OuterCutOff);
		}

		const auto& ibl_maps = scene_environment.GetIBLMaps();
		if (ibl_maps)
		{
			material.Set("irradianceMap", 6);
			if (auto texture = ibl_maps->IrradianceMap)
				texture->Bind(6);
			material.Set("prefilterMap", 7);
			if (auto texture = ibl_maps->PrefilteredMap)
				texture->Bind(7);
			material.Set("brdfLUT", 8);
			if (auto texture = ibl_maps->BrdfLUT)
				texture->Bind(8);

			float max_lod = (ibl_maps->PrefilterMipLevels > 0) ? float(ibl_maps->PrefilterMipLevels - 1) : 0.0f;
			material.Set("prefilterMaxLod", max_lod);
		}
		else
		{
			// TODO: No Environment Map
		}

		material.Set("envSettings.intensity", environment_settings.Intensity);
		material.Set("envSettings.rotation", environment_settings.Rotation);
		material.Set("envSettings.tint", environment_settings.Tint);
	}

	std::shared_ptr<Material> PBRPipeline::CreateSkyboxMaterial(const Environment& scene_environment)
	{
		auto material = Material::Create(m_shader_library.Get("Skybox"));


		material->GetShader()->Bind();

		material->Set("environmentMap", 0);
		const auto& skybox_map = scene_environment.GetSkyboxMap();
		if (skybox_map)
		{
			if (auto texture = skybox_map)
				texture->Bind(0);
		}

		return material;
	}

	std::shared_ptr<Shader> PBRPipeline::GetStandardShader()
	{
		return m_shader_library.Get("IgnisPBR");
	}

	std::shared_ptr<Shader> PBRPipeline::GetSkyboxShader()
	{
		return m_shader_library.Get("Skybox");
	}
}