#include "EventExample.h"

namespace ignis
{
	void EventExample()
	{
		EventDispatcher dispatcher;
		auto subscription = dispatcher.Subscribe<KeyPressedEvent>(
			[](KeyPressedEvent& event) {
				printf("Key Pressed: %d (repeat = %s)\n",
					event.GetKeyCode(),
					event.IsRepeat() ? "true" : "false");
				event.Handled = true;
			});

		while (true)
		{
			int ch = getchar();
			KeyPressedEvent e(ch);
			dispatcher.Dispatch(e);
		}
	}
}