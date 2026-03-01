#pragma once

#include "Ignis/Asset/Asset.h"
#include "ScriptBehaviour.h"

namespace ignis
{
	class Script : public Asset
	{
	public:
		Script(std::unique_ptr<ScriptBehaviour> behaviour)
			: m_behaviour(std::move(behaviour)) {
		}
		~Script() = default;

		AssetType GetAssetType() const override { return AssetType::Script; }

		const ScriptBehaviour& GetBehaviour() const { return *m_behaviour; }
		ScriptBehaviour& GetBehaviour() { return *m_behaviour; }

	private:
		std::unique_ptr<ScriptBehaviour> m_behaviour;
	};
}