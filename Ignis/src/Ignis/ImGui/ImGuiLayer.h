#pragma once

#include "Ignis/Core/Layer.h"

namespace ignis {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer() : Layer("ImGuiLayer") {}
		virtual ~ImGuiLayer() = default;

		virtual void Begin() = 0;
		virtual void End() = 0;

		void SetDarkTheme();
		void AllowInputEvents(bool allow_events);

		static std::unique_ptr<ImGuiLayer> Create();

	protected:
		bool m_block_events = true;
	};

}
