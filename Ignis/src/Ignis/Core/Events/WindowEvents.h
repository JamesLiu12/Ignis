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
	public:
		WindowResizeEvent(const unsigned width, const unsigned height) 
			: WindowEvent(WindowEventType::WindowResize, "WindowResize"), 
			m_width(width), m_height(height) {}
		virtual ~WindowResizeEvent() = default;

		unsigned GetWidth() const { return m_width; }
		unsigned GetHeight() const { return m_height; }
		
	private:
		unsigned m_width, m_height;
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
		WindowMovedEvent(const unsigned x_pos, const unsigned y_pos) 
			: WindowEvent(WindowEventType::WindowMoved, "WindowMoved"), 
			m_x_pos(x_pos), m_y_pos(y_pos) {}
		virtual ~WindowMovedEvent() = default;

		unsigned GetXPos() const { return m_x_pos; }
		unsigned GetYPos() const { return m_y_pos; }
		
	private:
		unsigned m_x_pos, m_y_pos;
	};
}