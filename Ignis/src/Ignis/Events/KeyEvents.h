#pragma once

#include "Event.h"

enum class KeyEvents
{
	KeyPressed,
	KeyReleased,
	KeyTyped
};

namespace ignis
{
	class KeyEvent : public Event<KeyEvents>
	{
	public:
		KeyEvent(const int keycode) 
			: Event(KeyEvents::KeyTyped, "KeyEvent"), m_keycode(keycode) {}
		virtual ~KeyEvent() = default;
		int GetKeyCode() const { return m_keycode; }

	protected:
		int m_keycode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const int keycode, const bool is_repeat = false) 
			: KeyEvent(keycode), m_is_repeat(is_repeat) 
		{
			m_type = KeyEvents::KeyPressed;
			m_name = "KeyPressedEvent";
		}
		virtual ~KeyPressedEvent() = default;

		bool IsRepeat() const { return m_is_repeat; }

	private:
		bool m_is_repeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public KeyReleasedEvent(const int keycode)
			: KeyEvent(keycode)
		{
			m_type = KeyEvents::KeyReleased;
			m_name = "KeyReleasedEvent";
		}
		virtual ~KeyReleasedEvent() = default;
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(const int keycode)
			: KeyEvent(keycode)
		{
			m_type = KeyEvents::KeyTyped;
			m_name = "KeyTypedEvent";
		}
		virtual ~KeyTypedEvent() = default;
	};
}