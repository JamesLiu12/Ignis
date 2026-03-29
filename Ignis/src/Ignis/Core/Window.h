#pragma once

#include "Ignis/Core/API.h"
#include "Ignis/Core/Events/Event.h"

#include <GLFW/glfw3.h>

namespace ignis
{
	class IGNIS_API Window
	{

	public:
		struct WindowSpecs
		{
			std::string Title;
			uint32_t    Width;
			uint32_t    Height;
			bool        VSync;

			WindowSpecs(const std::string& title = "Ignis Engine",
				uint32_t width = 1920,
				uint32_t height = 1080,
				bool vsync = true)
				: Title(title), Width(width), Height(height), VSync(vsync) {
			}
		};;

		using EventCallbackFn = std::function<void(EventBase&)>;

		~Window();

		void OnUpdate();

		uint32_t GetWidth() const { return m_data.Width; }
		uint32_t GetHeight() const { return m_data.Height; }

		uint32_t GetFramebufferWidth() const { return m_data.FramebufferWidth; }
		uint32_t GetFramebufferHeight() const { return m_data.FramebufferHeight; }

		float GetContentScaleX() const { return m_data.ContentScaleX; }
		float GetContentScaleY() const { return m_data.ContentScaleY; }

		void SetEventCallback(const EventCallbackFn& cb) { m_data.EventCallback = cb; }

		void SetTitle(const std::string& title);

		void SetVSync(bool enabled);
		bool IsVSync() const { return m_data.VSync; }

		void SetCursorMode(int mode);
		int GetCursorMode() const { return m_cursor_mode; }

		GLFWwindow* GetNativeWindow() const { return m_window; }

		static std::unique_ptr<Window> Create(const WindowSpecs& specs = WindowSpecs());

		Window(const WindowSpecs& specs = WindowSpecs());
	private:

		void Shutdown();
		void SetUpCallbacks();

		GLFWwindow* m_window = nullptr;

		int m_cursor_mode = GLFW_CURSOR_NORMAL;

		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			uint32_t FramebufferWidth, FramebufferHeight;
			float ContentScaleX, ContentScaleY;
			bool VSync;
			EventCallbackFn EventCallback;
		} m_data;
	};
}