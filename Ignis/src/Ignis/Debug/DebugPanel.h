#pragma once

#include "Ignis/Events/Event.h"

namespace ignis {

	class DebugPanel
	{
	public:
		virtual ~DebugPanel() = default;
		virtual void OnImGuiRender(bool& is_open) = 0;
		virtual void OnEvent(EventBase& event) {}
	};

}
