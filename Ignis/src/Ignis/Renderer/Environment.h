#pragma once

#include "Ignis/Asset/Asset.h"
#include <glm/glm.hpp>

namespace ignis
{
	struct IBLMaps
	{
		AssetHandle EnvironmentMap;
		AssetHandle IrradianceMap;
		AssetHandle PrefilteredMap;
		AssetHandle BrdfLUT;
		uint32_t PrefilterMipLevels = 1;
	};

	struct EnvironmentSettings
	{
		float Intensity = 1.0f;
		float Rotation = 0.0f;
		glm::vec3 Tint = { 1.0f, 1.0f, 1.0f };
		float SkyboxLod = 0.0f;
	};

	class Environment : public Asset
	{
	public:
		Environment() = default;
		~Environment() = default;

		const std::optional<AssetHandle> GetSkyboxMap() const { return m_skybox_map; }
		void SetSkyboxMap(AssetHandle skybox_map_handle) { m_skybox_map = skybox_map_handle; }

		void SetIBLMaps(const IBLMaps& maps) { m_ibl_maps = maps; }
		const std::optional<IBLMaps>& GetIBLMaps() const { return m_ibl_maps; }

	private:
		std::optional<AssetHandle> m_skybox_map;
		std::optional<IBLMaps> m_ibl_maps;
	};
}