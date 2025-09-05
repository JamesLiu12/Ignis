#pragma once

#include "Event.h"

namespace ignis
{
	enum class MouseEvents
	{
		MouseMoved,
		MouseScrolled,
		MouseButtonPressed,
		MouseButtonReleased
	};

	class MouseMovedEvent : public Event<MouseEvents>
	{
	public:
		MouseMovedEvent(const double x, const double y)
			: Event(MouseEvents::MouseMoved, "MouseMoved"), 
			m_x(x), m_y(y) {}

		double GetX() const { return m_x; }
		double GetY() const { return m_y; }

	private:
		double m_x, m_y;
	};

	class MouseScrolledEvent : public Event<MouseEvents>
	{
	public:
		MouseScrolledEvent(const double x_offset, const double y_offset)
			: Event(MouseEvents::MouseScrolled, "MouseScrolled"), 
			m_x_offset(x_offset), m_y_offset(y_offset) {}
	
		double GetXOffset() const { return m_x_offset; }
		double GetYOffset() const { return m_y_offset; }

	private:
		double m_x_offset, m_y_offset;
	};

	class MouseButtonEvent : public Event<MouseEvents>
	{
	public:
		MouseButtonEvent(const int button)
			: Event(MouseEvents::MouseButtonPressed, "MouseButtonEvent"), 
			m_button(button) {}

		int GetMouseButton() const { return m_button; }

	private:
		int m_button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(const int button)
			: MouseButtonEvent(button)
		{
			m_type = MouseEvents::MouseButtonPressed;
			m_name = "MouseButtonPressedEvent";
		}
		virtual ~MouseButtonPressedEvent() = default;
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
		public:
		MouseButtonReleasedEvent(const int button)
			: MouseButtonEvent(button)
		{
			m_type = MouseEvents::MouseButtonReleased;
			m_name = "MouseButtonReleasedEvent";
		}
		virtual ~MouseButtonReleasedEvent() = default;
	};
}