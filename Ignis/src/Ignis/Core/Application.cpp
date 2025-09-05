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
		auto subscription = dispatcher.Subscribe(KeyEvents::KeyPressed, [](Event<KeyEvents>& e) {
			auto& event = static_cast<KeyPressedEvent&>(e);
			printf("Key Pressed: %d (repeat = %s)\n", event.GetKeyCode(), event.IsRepeat() ? "true" : "false");
			e.Handled = true;
		});

		while (true)
		{
			int ch = getchar();
			auto e = KeyPressedEvent(ch);
			dispatcher.Dispatch(e);
		}
	}
}