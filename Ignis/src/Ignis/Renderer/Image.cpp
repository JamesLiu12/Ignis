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

			size_t size = width * height * BytesPerPixel(format);
			m_pixels.resize(size);
			std::memcpy(m_pixels.data(), data, size);
			m_loaded = true;
		}
	}

	std::shared_ptr<Image> Image::LoadFromFile(const std::filesystem::path& filepath, bool flip_vertical)
	{
		if (!std::filesystem::exists(filepath)) return nullptr;

		stbi_set_flip_vertically_on_load(flip_vertical);

		int width, height, channels;
		void* data = nullptr;
		ImageFormat format = ImageFormat::None;
		size_t data_size = 0;

		bool is_hdr = stbi_is_hdr(filepath.string().c_str());

		if (is_hdr)
		{
			float* float_data = stbi_loadf(filepath.string().c_str(), &width, &height, &channels, 0);
			data = float_data;

			if (float_data)
			{
				if (channels == 4) format = ImageFormat::RGBA32F;
				else if (channels == 3) format = ImageFormat::RGB32F;
				else if (channels == 1) format = ImageFormat::R32F;
				else {
					// Fallback: Force load as 4 channels if it's an odd format
					stbi_image_free(float_data);
					float_data = stbi_loadf(filepath.string().c_str(), &width, &height, &channels, 4);
					data = float_data;
					format = ImageFormat::RGBA32F;
					channels = 4;
				}

				data_size = width * height * channels * sizeof(float);
			}
		}
		else
		{
			stbi_uc* byte_data = stbi_load(filepath.string().c_str(), &width, &height, &channels, 0);
			data = byte_data;

			if (byte_data)
			{
				if (channels == 4) format = ImageFormat::RGBA8;
				else if (channels == 3) format = ImageFormat::RGB8;
				else if (channels == 1) format = ImageFormat::R8;
				else {
					// Fallback: Force load as 4 channels
					stbi_image_free(byte_data);
					byte_data = stbi_load(filepath.string().c_str(), &width, &height, &channels, 4);
					data = byte_data;
					format = ImageFormat::RGBA8;
					channels = 4;
				}

				data_size = width * height * channels * sizeof(stbi_uc);
			}
		}

		if (!data) return nullptr;

		auto image = std::make_shared<Image>();
		image->m_width = width;
		image->m_height = height;
		image->m_format = format;
		image->m_pixels.resize(data_size);

		std::memcpy(image->m_pixels.data(), data, data_size);
		image->m_loaded = true;

		stbi_image_free(data);
		return image;
	}

}