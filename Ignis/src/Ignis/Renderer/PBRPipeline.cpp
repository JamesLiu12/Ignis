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

		// --- Albedo ---
		auto albedo = AssetManager::GetAsset<Texture2D>(data.AlbedoMap);
		material->Set("material.albedoMap", albedo ? albedo : Renderer::GetWhiteTexture());
		material->Set("material.albedoColor", data.AlbedoColor);

		// --- Normal ---
		auto normal = AssetManager::GetAsset<Texture2D>(data.NormalMap);
		material->Set("material.normalMap", normal ? normal : Renderer::GetDefaultNormalTexture());

		// --- Metalness ---
		auto metalness = AssetManager::GetAsset<Texture2D>(data.MetalnessMap);
		material->Set("material.metallicMap", metalness ? metalness : Renderer::GetBlackTexture());
		material->Set("material.metallicValue", data.MetallicValue);

		// --- Roughness ---
		auto roughness = AssetManager::GetAsset<Texture2D>(data.RoughnessMap);
		material->Set("material.roughnessMap", roughness ? roughness : Renderer::GetDefaultRoughnessTexture());
		material->Set("material.roughnessValue", data.RoughnessValue);

		// --- Emissive ---
		auto emissive = AssetManager::GetAsset<Texture2D>(data.EmissiveMap);
		material->Set("material.emissiveMap", emissive ? emissive : Renderer::GetBlackTexture());
		material->Set("material.emissiveColor", data.EmissiveColor);
		material->Set("material.emissiveIntensity", data.EmissiveIntensity);

		// --- AO ---
		auto ao = AssetManager::GetAsset<Texture2D>(data.AOMap);
		material->Set("material.aoMap", ao ? ao : Renderer::GetWhiteTexture());

		// --- Clearcoat ---
		auto clearcoatMap = AssetManager::GetAsset<Texture2D>(data.ClearcoatMap);
		material->Set("material.clearcoatMap", clearcoatMap ? clearcoatMap : Renderer::GetWhiteTexture());
		material->Set("material.clearcoatFactor", data.ClearcoatFactor);

		auto clearcoatRoughnessMap = AssetManager::GetAsset<Texture2D>(data.ClearcoatRoughnessMap);
		material->Set("material.clearcoatRoughnessMap", clearcoatRoughnessMap ? clearcoatRoughnessMap : Renderer::GetWhiteTexture());
		material->Set("material.clearcoatRoughnessFactor", data.ClearcoatRoughnessFactor);

		auto clearcoatNormalMap = AssetManager::GetAsset<Texture2D>(data.ClearcoatNormalMap);
		material->Set("material.clearcoatNormalMap", clearcoatNormalMap ? clearcoatNormalMap : Renderer::GetDefaultNormalTexture());

		material->Set("uv_albedoMap", (int)data.AlbedoMapUVIndex);
		material->Set("uv_normalMap", (int)data.NormalMapUVIndex);
		material->Set("uv_metallicMap", (int)data.MetalnessMapUVIndex);
		material->Set("uv_roughnessMap", (int)data.RoughnessMapUVIndex);
		material->Set("uv_emissiveMap", (int)data.EmissiveMapUVIndex);
		material->Set("uv_aoMap", (int)data.AOMapUVIndex);
		material->Set("uv_clearcoatMap", (int)data.ClearcoatMapUVIndex);
		material->Set("uv_clearcoatRoughnessMap", (int)data.ClearcoatRoughnessMapUVIndex);
		material->Set("uv_clearcoatNormalMap", (int)data.ClearcoatNormalMapUVIndex);

		material->Set("ch_metallic", data.MetallicChannel);
		material->Set("ch_roughness", data.RoughnessChannel);

		material->Set("uvT_albedoMap", data.AlbedoMapUVTransform.ToMatrix());
		material->Set("uvT_normalMap", data.NormalMapUVTransform.ToMatrix());
		material->Set("uvT_metallicMap", data.MetalnessMapUVTransform.ToMatrix());
		material->Set("uvT_roughnessMap", data.RoughnessMapUVTransform.ToMatrix());
		material->Set("uvT_emissiveMap", data.EmissiveMapUVTransform.ToMatrix());
		material->Set("uvT_aoMap", data.AOMapUVTransform.ToMatrix());
		material->Set("uvT_clearcoatMap", data.ClearcoatMapUVTransform.ToMatrix());
		material->Set("uvT_clearcoatRoughnessMap", data.ClearcoatRoughnessMapUVTransform.ToMatrix());
		material->Set("uvT_clearcoatNormalMap", data.ClearcoatNormalMapUVTransform.ToMatrix());

		return material;
	}

	void PBRPipeline::ApplyEnvironment(Material& material,
		const Environment& scene_environment,
		const EnvironmentSettings& environment_settings,
		const LightEnvironment& light_environment)
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
			if (ibl_maps->IrradianceMap)
				material.Set("irradianceMap", ibl_maps->IrradianceMap);

			if (ibl_maps->PrefilteredMap)
				material.Set("prefilterMap", ibl_maps->PrefilteredMap);

			material.Set("brdfLUT", ibl_maps->BrdfLUT ? ibl_maps->BrdfLUT : m_brdf_lut_texture);

			float max_lod = (ibl_maps->PrefilterMipLevels > 0)
				? float(ibl_maps->PrefilterMipLevels - 1)
				: 0.0f;
			material.Set("prefilterMaxLod", max_lod);
		}
		else
		{
			material.Set("irradianceMap", Renderer::GetBlackTextureCube());
			material.Set("prefilterMap", Renderer::GetBlackTextureCube());
			material.Set("brdfLUT", m_brdf_lut_texture);
			material.Set("prefilterMaxLod", 0.0f);
		}

		material.Set("envSettings.intensity", environment_settings.Intensity);
		material.Set("envSettings.rotation", environment_settings.Rotation);
		material.Set("envSettings.tint", environment_settings.Tint);
	}

	std::shared_ptr<Material> PBRPipeline::CreateSkyboxMaterial(const Environment& scene_environment, 
		const EnvironmentSettings& environment_settings)
	{
		auto material = Material::Create(m_shader_library.Get("Skybox"));

		const auto& skybox_map = scene_environment.GetSkyboxMap();
		if (skybox_map)
			material->Set("environmentMap", skybox_map);

		material->Set("envRotation", environment_settings.Rotation);
		material->Set("envIntensity", environment_settings.Intensity);
		material->Set("envTint", environment_settings.Tint);

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