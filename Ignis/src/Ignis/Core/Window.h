#pragma once

#include "Ignis/Events/Event.h"
#include "Ignis/Events/WindowEvents.h"

namespace ignis
{
	class Window
	{

	public:
		struct WindowProps {
			std::string Title = "Ignis Engine";
			uint32_t    Width = 1920;
			uint32_t    Height = 1080;
			bool        VSync = true;
		};

		using EventCallbackFn = std::function<void(EventBase&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& cb) = 0;

		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		static std::unique_ptr<Window> Create(const WindowProps& props = {});

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

	protected:
		Window() = default;
	};
}