#pragma once

#include "KeyCodes.h"

#include <utility>

namespace ignis
{
	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);
		static bool IsKeyReleased(KeyCode key);

		static bool IsMouseButtonPressed(MouseButton button);
		static bool IsMouseButtonReleased(MouseButton button);
		static std::pair<double, double> GetMousePosition();
		static double GetMouseX();
		static double GetMouseY();
	private:
		Input();
	};
}
