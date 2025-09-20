#pragma once

#include "Ignis/Renderer/RendererContext.h"

namespace ignis
	{
	class GLRendererContext : public RendererContext
	{
	public:
		void Init() override;
	};
}