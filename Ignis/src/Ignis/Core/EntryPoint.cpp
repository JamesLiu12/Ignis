#include "EntryPoint.h"

int main(int argc, char** argv)
{
	puts("Hi, Ignis");
	auto app = ignis::CreateApplication();
	app->Run();
	return 0;
}