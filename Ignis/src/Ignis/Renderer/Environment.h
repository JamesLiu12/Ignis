#pragma once

#include "Ignis/Asset/Asset.h"
#include <glm/glm.hpp>

namespace ignis
{
	struct IBLMaps
	{
		AssetHandle IrradianceMap;
		AssetHandle PrefilteredMap;
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

		void SetIBLMaps(const IBLMaps& maps) { m_ibl_maps = maps; }

		const std::optional<IBLMaps>& GetIBLMaps() const { return m_ibl_maps; }

	private:
		std::optional<IBLMaps> m_ibl_maps;
	};
}