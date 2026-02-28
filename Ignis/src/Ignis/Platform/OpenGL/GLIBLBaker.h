#include "Ignis/Renderer/IBLBaker.h"

namespace ignis
{
	class GLRenderer;

	class GLIBLBaker : public IBLBaker
	{
	public:
		GLIBLBaker(Renderer& renderer)
			: m_renderer(renderer) {
		}

		IBLBakeResult BakeFromEquirectangular(
			const Image& hdr_image,
			const IBLBakeSettings& settings = {}
		) const override;

	private:
		Renderer& m_renderer;
	};
}