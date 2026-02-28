#pragma once

#include "Ignis/Asset/Asset.h"
#include <glm/glm.hpp>

namespace ignis
{
	class Texture2D;
	class TextureCube;

	struct IBLMaps
	{
		std::shared_ptr<TextureCube> IrradianceMap;
		std::shared_ptr<TextureCube> PrefilteredMap;
		std::shared_ptr<Texture2D>   BrdfLUT;
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

		const std::shared_ptr<TextureCube> GetSkyboxMap() const { return m_skybox_map; }
		void SetSkyboxMap(std::shared_ptr<TextureCube> skybox_map) { m_skybox_map = skybox_map; }

		void SetIBLMaps(const IBLMaps& maps) { m_ibl_maps = maps; }
		const std::optional<IBLMaps>& GetIBLMaps() const { return m_ibl_maps; }

	private:
		std::shared_ptr<TextureCube> m_skybox_map;
		std::optional<IBLMaps> m_ibl_maps;
	};
}