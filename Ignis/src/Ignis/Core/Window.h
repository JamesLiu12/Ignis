#pragma once

#include "pch.h"

#include "Ignis/Events/Event.h"
#include "Ignis/Events/WindowEvents.h"

namespace ignis
{
	class Window
	{
		struct WindowProps
		{
			std::string Title;
			uint32_t Width;
			uint32_t Height;

			WindowProps(const std::string& title = "Ignis Engine",
				uint32_t width = 1920,
				uint32_t height = 1080)
				: Title(title), Width(width), Height(height) { }
		};

		class Window
		{
		public:
			virtual ~Window() = default;
			virtual void OnUpdate() = 0;

			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;

			static std::unique_ptr<Window> Create(const WindowProps& props = WindowProps());
		};
	};
}