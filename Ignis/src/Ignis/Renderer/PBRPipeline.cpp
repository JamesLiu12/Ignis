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

		auto albedo = AssetManager::GetAsset<Texture2D>(data.AlbedoMap);
		material->Set("material.albedoMap", albedo ? albedo : Renderer::GetWhiteTexture());

		auto normal = AssetManager::GetAsset<Texture2D>(data.NormalMap);
		material->Set("material.normalMap", normal ? normal : Renderer::GetDefaultNormalTexture());

		auto metalness = AssetManager::GetAsset<Texture2D>(data.MetalnessMap);
		material->Set("material.metallicMap", metalness ? metalness : Renderer::GetBlackTexture());

		auto roughness = AssetManager::GetAsset<Texture2D>(data.RoughnessMap);
		material->Set("material.roughnessMap", roughness ? roughness : Renderer::GetDefaultRoughnessTexture());

		auto emissive = AssetManager::GetAsset<Texture2D>(data.EmissiveMap);
		material->Set("material.emissiveMap", emissive ? emissive : Renderer::GetBlackTexture());

		auto ao = AssetManager::GetAsset<Texture2D>(data.AOMap);
		material->Set("material.aoMap", ao ? ao : Renderer::GetWhiteTexture());

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
			// TODO: No Environment Map
		}

		material.Set("envSettings.intensity", environment_settings.Intensity);
		material.Set("envSettings.rotation", environment_settings.Rotation);
		material.Set("envSettings.tint", environment_settings.Tint);
	}

	std::shared_ptr<Material> PBRPipeline::CreateSkyboxMaterial(const Environment& scene_environment)
	{
		auto material = Material::Create(m_shader_library.Get("Skybox"));

		const auto& skybox_map = scene_environment.GetSkyboxMap();
		if (skybox_map)
			material->Set("environmentMap", skybox_map);

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