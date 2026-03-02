#pragma once

#include "ScriptBehaviour.h"

namespace ignis
{
	class Script
	{
	public:
		Script(std::unique_ptr<ScriptBehaviour> behaviour)
			: m_behaviour(std::move(behaviour)) {
		}
		~Script() = default;

		Script(const Script&) = delete;
		Script& operator=(const Script&) = delete;

		Script(Script&&) noexcept = default;
		Script& operator=(Script&&) noexcept = default;

		const ScriptBehaviour& GetBehaviour() const { return *m_behaviour; }
		ScriptBehaviour& GetBehaviour() { return *m_behaviour; }

	private:
		std::unique_ptr<ScriptBehaviour> m_behaviour;
	};
}