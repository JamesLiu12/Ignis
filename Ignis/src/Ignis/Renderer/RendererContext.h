#pragma once

namespace ignis
{
	class RendererContext
	{
	public:
		virtual ~RendererContext() = default;

		virtual void Init() = 0;

		static std::unique_ptr<RendererContext> Create();
	};
}