#pragma once

#include "Ignis/Core/API.h"
#include "KeyCodes.h"

#include <utility>

namespace ignis
{
	class IGNIS_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);
		static bool IsKeyReleased(KeyCode key);

		static bool IsMouseButtonPressed(MouseButton button);
		static bool IsMouseButtonReleased(MouseButton button);
		static std::pair<double, double> GetMousePosition();
		static double GetMouseX();
		static double GetMouseY();

		static void ShowCursor();
		static void HideCursor();
		static void LockCursor();
		static bool IsCursorLocked();
		static bool IsCursorVisible();

		static bool ConsumeCursorJustLocked();
	private:
		Input();

		inline static bool s_cursor_just_locked = false;
	};
}
