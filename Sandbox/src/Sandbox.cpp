#include "Sandbox.h"
#include "SandboxLayer.h"

std::unique_ptr<ignis::Application> ignis::Application::Create()
{
	return std::make_unique<Sandbox>();
}

Sandbox::Sandbox()
{
	PushLayer(std::make_unique<SandBoxLayer>(GetRenderer()));
}