#include "EntryPoint.h"
#include "Ignis/Core/Log.h"

int main(int argc, char** argv)
{
	puts("Hi, Ignis");
	{
		auto app = ignis::Application::Create();
		app->Run();
	}
	ignis::Log::Shutdown();
	return 0;
}