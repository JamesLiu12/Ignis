#pragma once

#include "Ignis/ImGui/ImGuiLayer.h"

struct GLFWwindow;

namespace ignis {

	class GlfwImGuiLayer : public ImGuiLayer
	{
	public:
		GlfwImGuiLayer();
		virtual ~GlfwImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(EventBase& e) override;

		void Begin() override;
		void End() override;

	private:
		float m_time = 0.0f;
	};

}
