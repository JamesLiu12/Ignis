#pragma once

#include "Event.h"

enum class KeyEventType
{
	None = 0,
	KeyPressed,
	KeyReleased,
	KeyTyped
};

namespace ignis
{
	class KeyEvent : public Event<KeyEventType>
	{
	public:
		KeyEvent(KeyEventType type, const std::string& name, const int keycode) 
			: Event(type, name), m_keycode(keycode) {}
		virtual ~KeyEvent() = default;
		int GetKeyCode() const { return m_keycode; }

	protected:
		int m_keycode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const int keycode, const bool is_repeat = false) 
			: KeyEvent(KeyEventType::KeyPressed, "KeyPressedEvent", keycode), 
			m_is_repeat(is_repeat) {}
		virtual ~KeyPressedEvent() = default;

		bool IsRepeat() const { return m_is_repeat; }

	private:
		bool m_is_repeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(const int keycode)
			: KeyEvent(KeyEventType::KeyReleased, "KeyReleasedEvent", keycode) {}
		virtual ~KeyReleasedEvent() = default;
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(const int keycode)
			: KeyEvent(KeyEventType::KeyTyped, "KeyTypedEvent", keycode) {}
		virtual ~KeyTypedEvent() = default;
	};
}