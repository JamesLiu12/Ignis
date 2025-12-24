#include "Image.h"
#include <stb_image.h>

namespace ignis
{
	Image::Image(uint32_t width, uint32_t height, ImageFormat format, const void* data)
	{
		if (data) {
			m_width = width;
			m_height = height;
			m_format = format;
			m_pixels.resize(width * height * Channels(format));
			std::memcpy(m_pixels.data(), data, width * height * Channels(format));
			m_loaded = true;
		}
	}

	std::shared_ptr<Image> Image::LoadFromFile(const std::filesystem::path& filepath, bool flip_vertical)
	{
		stbi_set_flip_vertically_on_load(flip_vertical);

		int width, height, channels;

		stbi_uc* data = stbi_load(filepath.string().c_str(), &width, &height, &channels, 0);

		if (!data) return nullptr;

		auto image = std::make_shared<Image>();
		image->m_width = width;
		image->m_height = height;

		if (channels == 4)
		{
			image->m_format = ImageFormat::RGBA8;
		}
		else if (channels == 3)
		{
			image->m_format = ImageFormat::RGB8;
		}
		else if (channels == 1)
		{
			image->m_format = ImageFormat::R8;
		}
		else
		{
			stbi_image_free(data);
			data = stbi_load(filepath.string().c_str(), &width, &height, &channels, 4);
			image->m_format = ImageFormat::RGBA8;
		}

		size_t size = width * height * channels;
		image->m_pixels.resize(size);
		memcpy(image->m_pixels.data(), data, size);

		stbi_image_free(data);
		return image;
	}

}