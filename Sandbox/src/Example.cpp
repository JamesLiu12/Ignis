#include "Ignis.h"
#include <memory>

int main()
{
	std::unique_ptr<Ignis::Application> app = std::make_unique<Ignis::Application>();
	app->Run();
	return 0;
}