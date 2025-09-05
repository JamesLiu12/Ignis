#pragma once

#include "Event.h"

namespace ignis
{
	enum class WindowEvents
	{
		WindowClose,
		WindowResize,
		WindowFocus,
		WindowLostFocus,
		WindowMoved
	};

	class WindowCloseEvent : public Event<WindowEvents>
	{
	public:
		WindowCloseEvent() : Event(WindowEvents::WindowClose, "WindowClose") {}
		virtual ~WindowCloseEvent() = default;
	};

	class WindowResizeEvent : public Event<WindowEvents>
	{
		WindowCloseEvent() : Event(WindowEvents::WindowClose, "WindowClose") {}
		virtual ~WindowResizeEvent() = default;
		unsigned width, height;
	};

	class WindowFocusEvent : public Event<WindowEvents>
	{
		public:
		WindowFocusEvent() : Event(WindowEvents::WindowFocus, "WindowFocus") {}
		virtual ~WindowFocusEvent() = default;
	};

	class WindowLostFocusEvent : public Event<WindowEvents>
	{
		public:
		WindowLostFocusEvent() : Event(WindowEvents::WindowLostFocus, "WindowLostFocus") {}
		virtual ~WindowLostFocusEvent() = default;
	};

	class WindowMovedEvent : public Event<WindowEvents>
	{
		public:
		WindowMovedEvent() : Event(WindowEvents::WindowMoved, "WindowMoved") {}
		virtual ~WindowMovedEvent() = default;
		unsigned xPos, yPos;
	};
}