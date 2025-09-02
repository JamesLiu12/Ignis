#include "EntryPoint.h"

int main(int argc, int** argv)
{
	puts("Hi, Ignis");
	auto app = Ignis::CreateApplication();
	app->Run();
	return 0;
}