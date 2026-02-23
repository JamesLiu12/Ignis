#include "GLTexture.h"
#include "Ignis/Renderer/Image.h"
#include "GLUtils.h"
#include <glad/glad.h>

namespace ignis
{
	void ApplyTextureParameters(const TextureSpecs& specs)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, utils::ToGLTextureWrap(specs.WrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, utils::ToGLTextureWrap(specs.WrapT));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, utils::ToGLMinFilter(specs.MinFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, utils::ToGLMagFilter(specs.MagFilter));
	}

	GLTexture2D::GLTexture2D(const TextureSpecs& specs, ImageFormat source_format, std::span<const std::byte> data)
		: m_specs(specs)
	{
		if (m_specs.Width == 0 || m_specs.Height == 0)
		{
			Log::CoreError("Texture dimensions must be non-zero for manual texture creation.");
			return;
		}

		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);

		ApplyTextureParameters(m_specs);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		SetData(source_format, data);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	GLTexture2D::GLTexture2D(const TextureSpecs& specs)
		: m_specs(specs)
	{
		if (m_specs.Width == 0 || m_specs.Height == 0)
		{
			Log::CoreError("Texture dimensions must be non-zero for manual texture creation.");
			return;
		}

		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);

		ApplyTextureParameters(m_specs);

		const GLenum internal_format = utils::ToGLTextureFormat(m_specs.Format);
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_specs.Width, m_specs.Height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void GLTexture2D::SetData(ImageFormat source_format, std::span<const std::byte> data) const
	{
		const GLenum internal_format = utils::ToGLTextureFormat(m_specs.Format);
		const GLenum data_format = utils::ToGLImageFormat(source_format);
		const GLenum data_type = utils::ToGLDataType(source_format);

		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_specs.Width, m_specs.Height, 0, data_format, data_type, static_cast<const void*>(data.data()));

		if (m_specs.GenMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	void GLTexture2D::Bind(uint32_t unit) const
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, m_id);
	}

	void GLTexture2D::UnBind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}



	GLTextureCube::GLTextureCube(const TextureSpecs& specs, ImageFormat source_format, std::span<const std::byte> data)
		: m_specs(specs)
	{
		if (m_specs.Width == 0 || m_specs.Height == 0)
		{
			Log::CoreError("CubeMap dimensions must be non-zero.");
			return;
		}

		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, utils::ToGLMinFilter(m_specs.MinFilter));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, utils::ToGLMagFilter(m_specs.MagFilter));

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		SetData(source_format, data);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	GLTextureCube::GLTextureCube(const TextureSpecs& specs)
		: m_specs(specs)
	{
		if (m_specs.Width == 0 || m_specs.Height == 0)
		{
			Log::CoreError("CubeMap dimensions must be non-zero.");
			return;
		}

		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, utils::ToGLMinFilter(m_specs.MinFilter));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, utils::ToGLMagFilter(m_specs.MagFilter));

		const GLenum internal_format = utils::ToGLTextureFormat(m_specs.Format);

		// Allocate storage for all 6 faces without initializing data
		for (int i = 0; i < 6; ++i)
		{
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				internal_format,
				m_specs.Width,
				m_specs.Height,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				nullptr
			);
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void GLTextureCube::SetData(ImageFormat source_format, std::span<const std::byte> data) const
	{
		uint32_t face_size = m_specs.Width * m_specs.Height * BytesPerPixel(source_format);

		if (data.size() != face_size * 6)
		{
			Log::CoreError("TextureCube data size mismatch! Expected {} bytes (6 faces), got {}.", face_size * 6, data.size());
			return;
		}

		const GLenum internal_format = utils::ToGLTextureFormat(m_specs.Format);
		const GLenum data_format = utils::ToGLImageFormat(source_format);
		const GLenum data_type = utils::ToGLDataType(source_format);

		const std::byte* raw_data = data.data();

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		for (int i = 0; i < 6; ++i)
		{
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				internal_format,
				m_specs.Width,
				m_specs.Height,
				0,
				data_format,
				data_type,
				raw_data + (i * face_size)
			);
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		if (m_specs.GenMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}
	}

	void GLTextureCube::Bind(uint32_t unit) const
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
	}

	void GLTextureCube::UnBind() const
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}