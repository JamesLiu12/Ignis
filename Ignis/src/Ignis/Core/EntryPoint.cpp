#include "EntryPoint.h"

int main(int argc, char** argv)
{
	puts("Hi, Ignis");
	auto app = ignis::Application::Create();
	app->Run();
	return 0;
}