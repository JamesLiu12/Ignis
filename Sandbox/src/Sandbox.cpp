#include <memory>
#include "Sandbox.h"

std::unique_ptr<Ignis::Application> Ignis::CreateApplication()
{
	return std::make_unique<Sandbox>();
}