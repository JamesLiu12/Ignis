#pragma once

#include "Ignis/Core/Window.h"
#include <GLFW/glfw3.h>

namespace ignis
{
	class GlfwWindow : public Window
	{
	public:
		GlfwWindow(const WindowProps& props);
		~GlfwWindow() override;

		void OnUpdate() override;

		uint32_t GetWidth() const override { return m_data.Width; }
		uint32_t GetHeight() const override { return m_data.Height; }

		void SetEventCallback(const EventCallbackFn& cb) override { m_data.EventCallback = cb; }

		void SetVSync(bool enabled) override;
		bool IsVSync() const override { return m_data.VSync; }

		void* GetNativeHandle() const { return m_window; }

	private:
		void Shutdown();
		void SetUpCallbacks();

		GLFWwindow* m_window = nullptr;

		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool VSync;
			EventCallbackFn EventCallback;
		} m_data;
	};
}