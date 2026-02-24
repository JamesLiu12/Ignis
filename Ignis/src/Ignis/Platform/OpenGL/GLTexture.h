#include "Ignis/Renderer/Texture.h"

namespace ignis
{
	class GLTexture2D : public Texture2D
	{
	public:
		GLTexture2D(const TextureSpecs& specs, ImageFormat source_format, std::span<const std::byte> data);
		GLTexture2D(const TextureSpecs& specs);
		~GLTexture2D() override = default;

		uint32_t GetWidth() const override { return m_specs.Width; }
		uint32_t GetHeight() const override { return m_specs.Height; }

		void SetData(ImageFormat source_format, std::span<const std::byte> data) const override;

		void Bind(uint32_t unit) const override;
		void UnBind() const override;

		uint32_t GetRendererID() const override { return m_id; }

	private:
		uint32_t m_id = 0;
		TextureSpecs m_specs;

		friend class GLFramebuffer;
	};

	class GLTextureCube : public TextureCube
	{
	public:
		GLTextureCube (const TextureSpecs& specs, ImageFormat source_format, std::span<const std::byte> data);
		GLTextureCube(const TextureSpecs& specs);
		~GLTextureCube() override = default;

		uint32_t GetWidth() const override { return m_specs.Width; }
		uint32_t GetHeight() const override { return m_specs.Height; }

		void SetData(ImageFormat source_format, std::span<const std::byte> data) const override;

		void Bind(uint32_t unit) const override;
		void UnBind() const override;

		// OpenGL-specific method to get texture ID
		uint32_t GetOpenGLTextureID() const { return m_id; }

	private:
		uint32_t m_id = 0;
		TextureSpecs m_specs;
	};
}