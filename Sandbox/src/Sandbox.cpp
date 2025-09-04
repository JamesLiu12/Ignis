#include "Sandbox.h"

std::unique_ptr<ignis::Application> ignis::CreateApplication()
{
	return std::make_unique<Sandbox>();
}