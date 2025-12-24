#pragma once

namespace ignis
{
	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F
	};

	constexpr std::uint32_t Channels(ImageFormat format) noexcept {
		switch (format) {
		case ImageFormat::R8:		return 1;
		case ImageFormat::RGB8:     return 3;
		case ImageFormat::RGBA8:    return 4;
		case ImageFormat::RGBA32F:  return 4;
		default:                    return 0;
		}
		return 0;
	}

	class Image
	{
	public:
		Image() = default;
		Image(uint32_t width, uint32_t height, ImageFormat format, const void* data = nullptr);

		static std::shared_ptr<Image> LoadFromFile(const std::filesystem::path& filepath, bool flip_vertical = true);

		uint32_t GetWidth() const noexcept { return m_width; }
		uint32_t GetHeight() const noexcept { return m_height; }
		ImageFormat GetFormat() const noexcept { return m_format; }
		bool IsLoaded() const noexcept { return m_loaded; }

		std::span<const std::byte> GetPixels() const noexcept { return m_pixels; }

	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		ImageFormat m_format = ImageFormat::None;
		std::vector<std::byte> m_pixels;
		bool m_loaded = false;
	};
}