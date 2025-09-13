#pragma once

#include "Event.h"

namespace ignis
{
	enum class MouseEventType
	{
		None = 0,
		MouseMoved,
		MouseScrolled,
		MouseButtonPressed,
		MouseButtonReleased
	};

	class MouseEvent : public Event<MouseEventType>
	{
		public:
		MouseEvent(MouseEventType type, const std::string& name) : Event(type, name) {}
		virtual ~MouseEvent() = default;
	};

	class MouseMovedEvent : public MouseEvent
	{
	public:
		MouseMovedEvent(const double x, const double y)
			: MouseEvent(MouseEventType::MouseMoved, "MouseMoved"),
			m_x(x), m_y(y) {}

		double GetX() const { return m_x; }
		double GetY() const { return m_y; }

	private:
		double m_x, m_y;
	};

	class MouseScrolledEvent : public MouseEvent
	{
	public:
		MouseScrolledEvent(const double x_offset, const double y_offset)
			: MouseEvent(MouseEventType::MouseScrolled, "MouseScrolled"), 
			m_x_offset(x_offset), m_y_offset(y_offset) {}
	
		double GetXOffset() const { return m_x_offset; }
		double GetYOffset() const { return m_y_offset; }

	private:
		double m_x_offset, m_y_offset;
	};

	class MouseButtonEvent : public MouseEvent
	{
	public:
		MouseButtonEvent(const int button)
			: MouseEvent(MouseEventType::MouseButtonPressed, "MouseButtonEvent"), 
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
			m_type = MouseEventType::MouseButtonPressed;
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
			m_type = MouseEventType::MouseButtonReleased;
			m_name = "MouseButtonReleasedEvent";
		}
		virtual ~MouseButtonReleasedEvent() = default;
	};
}