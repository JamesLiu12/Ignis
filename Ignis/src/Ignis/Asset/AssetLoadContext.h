#pragma once
#include "Ignis/Renderer/IBLBaker.h"

namespace ignis
{
	struct AssetLoadContext
	{
		std::shared_ptr<IBLBaker> IBLBakerService = nullptr;
	};
}