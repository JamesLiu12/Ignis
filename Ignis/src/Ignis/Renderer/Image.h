#pragma once

namespace ignis
{
	enum class ImageFormat
	{
		RED,
		RG,
		RGB,
		BGR,
		RGBA,
		BGRA
	};

	constexpr std::uint32_t Channels(ImageFormat format) noexcept {
		switch (format) {
		case ImageFormat::RED:  return 1;
		case ImageFormat::RG:   return 2;
		case ImageFormat::RGB:
		case ImageFormat::BGR:  return 3;
		case ImageFormat::RGBA:
		case ImageFormat::BGRA: return 4;
		default:                return 0;
		}
		return 0;
	}

	class Image
	{
	public:
		Image(const std::string& filepath, ImageFormat image_format = ImageFormat::RGBA, bool flip_vertical = true);

		uint32_t GetWidth() const noexcept { return m_width; }
		uint32_t GetHeight() const noexcept { return m_height; }
		ImageFormat GetFormat() const noexcept { return m_format; }
		bool IsLoaded() const noexcept { return m_loaded; }

		std::span<const std::byte> GetPixels() const noexcept { return m_pixels; }

	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		ImageFormat m_format;
		std::vector<std::byte> m_pixels;
		bool m_loaded = false;
	};;
}