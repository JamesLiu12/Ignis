#include "Application.h"

#include "Ignis/Events/Event.h"
#include "Ignis/Events/KeyEvents.h"

namespace ignis {

	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		EventDispatcher<KeyEvents> dispatcher;
		auto subscription = dispatcher.Subscribe<KeyPressedEvent>(KeyEvents::KeyPressed,
			[](KeyPressedEvent& event) {
				printf("Key Pressed: %d (repeat = %s)\n",
					event.GetKeyCode(),
					event.IsRepeat() ? "true" : "false");
				event.Handled = true;
			});

		while (true)
		{
			int ch = getchar();
			dispatcher.Dispatch<KeyPressedEvent>(ch, false);
		}
	}
}