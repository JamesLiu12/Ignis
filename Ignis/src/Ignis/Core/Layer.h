#pragma once

#include "Ignis/Events/Event.h"
#include "pch.h"

namespace ignis {

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(EventBase& event) {}

		const std::string& GetName() const { return m_debug_name; }

	protected:
		std::string m_debug_name;
	};

}
