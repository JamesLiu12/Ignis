#pragma once

#include "Event.h"

namespace ignis
{
	enum class WindowEventType
	{
		None = 0,
		WindowClose,
		WindowResize,
		WindowFocus,
		WindowLostFocus,
		WindowMoved
	};

	class WindowEvent : public Event<WindowEventType>
	{
		public:
		WindowEvent(WindowEventType type, const std::string& name) : Event(type, name) {}
		virtual ~WindowEvent() = default;
	};

	class WindowCloseEvent : public WindowEvent
	{
	public:
		WindowCloseEvent() : WindowEvent(WindowEventType::WindowClose, "WindowClose") {}
		virtual ~WindowCloseEvent() = default;
	};

	class WindowResizeEvent : public WindowEvent
	{
		WindowResizeEvent() : WindowEvent(WindowEventType::WindowResize, "WindowResize") {}
		virtual ~WindowResizeEvent() = default;
		unsigned width, height;
	};

	class WindowFocusEvent : public WindowEvent
	{
		public:
		WindowFocusEvent() : WindowEvent(WindowEventType::WindowFocus, "WindowFocus") {}
		virtual ~WindowFocusEvent() = default;
	};

	class WindowLostFocusEvent : public WindowEvent
	{
		public:
		WindowLostFocusEvent() : WindowEvent(WindowEventType::WindowLostFocus, "WindowLostFocus") {}
		virtual ~WindowLostFocusEvent() = default;
	};

	class WindowMovedEvent : public WindowEvent
	{
		public:
		WindowMovedEvent() : WindowEvent(WindowEventType::WindowMoved, "WindowMoved") {}
		virtual ~WindowMovedEvent() = default;
		unsigned xPos, yPos;
	};
}