#include "Sandbox.h"

std::unique_ptr<ignis::Application> ignis::Application::Create()
{
	return std::make_unique<Sandbox>();
}