#include "Ignis/Renderer/Texture.h"

namespace ignis
{
	class GLTexture2D : public Texture2D
	{
	public:
		GLTexture2D(const TextureSpecs& specs, const std::string& filepath, bool flip_vertical);
		GLTexture2D(const TextureSpecs& specs, std::span<const std::byte> data);
		~GLTexture2D() override = default;

		uint32_t GetWidth() const override { return m_specs.Width; }
		uint32_t GetHeight() const override { return m_specs.Height; }

		void Bind(uint32_t unit) const override;
		void UnBind() const override;

	private:
		uint32_t m_id = 0;

		TextureSpecs m_specs;
	};
}