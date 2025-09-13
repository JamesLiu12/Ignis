#pragma once

#include <GLFW/glfw3.h>

#include "Ignis/Core/Events/Event.h"

namespace ignis
{
	class Window
	{

	public:
		struct WindowSpecs
		{
			std::string Title = "Ignis Engine";
			uint32_t    Width = 1920;
			uint32_t    Height = 1080;
			bool        VSync = true;
		};;

		using EventCallbackFn = std::function<void(EventBase&)>;

		~Window();

		void OnUpdate();

		uint32_t GetWidth() const { return m_data.Width; }
		uint32_t GetHeight() const { return m_data.Height; }

		void SetEventCallback(const EventCallbackFn& cb) { m_data.EventCallback = cb; }

		void SetVSync(bool enabled);
		bool IsVSync() const { return m_data.VSync; }

		GLFWwindow* GetNativeWindow() const { return m_window; }

		static std::unique_ptr<Window> Create(const WindowSpecs& specs = WindowSpecs());

		Window(const WindowSpecs& specs = WindowSpecs());
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