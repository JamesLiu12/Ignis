#include "Image.h"
#include <stb_image.h>

namespace ignis
{
	Image::Image(const std::string& filepath, ImageFormat image_format, bool flip_vertical)
		: m_format(image_format)
	{
		stbi_set_flip_vertically_on_load(flip_vertical);

		int width = 0;
		int height = 0;
		int num_channels = 0;

		unsigned char* raw_pixels = stbi_load(
			VFS::Resolve(filepath).string().c_str(), 
			&width, 
			&height, 
			&num_channels, 
			static_cast<int>(Channels(image_format))
		);

		if (!raw_pixels) {
			m_loaded = false;
			return;
		}

		m_width = static_cast<uint32_t>(width);
		m_height = static_cast<uint32_t>(height);

		const std::size_t total_bytes =
			static_cast<std::size_t>(m_width) *
			static_cast<std::size_t>(m_height) *
			static_cast<std::size_t>(Channels(m_format));

		m_pixels.resize(total_bytes);
		std::memcpy(m_pixels.data(), raw_pixels, total_bytes);

		m_loaded = true;

		stbi_image_free(raw_pixels);
	}
}