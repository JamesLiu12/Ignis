#include "GLTexture.h"
#include "Ignis/Renderer/Image.h"
#include <glad/glad.h>

namespace ignis
{
	GLenum ToGLTextureWrap(TextureWrap wrap)
	{
		switch (wrap)
		{
		case TextureWrap::Repeat:          return GL_REPEAT;
		case TextureWrap::MirroredRepeat:  return GL_MIRRORED_REPEAT;
		case TextureWrap::ClampToEdge:     return GL_CLAMP_TO_EDGE;
		case TextureWrap::ClampToBorder:   return GL_CLAMP_TO_BORDER;
		default:
			Log::CoreWarn("Unknown TextureWrap value ({}). Falling back to GL_REPEAT.", static_cast<int>(wrap));
			return GL_REPEAT;
		}
	}

	GLenum ToGLMinFilter(TextureFilter filter)
	{
		switch (filter)
		{
		case TextureFilter::Nearest:               return GL_NEAREST;
		case TextureFilter::Linear:                return GL_LINEAR;
		case TextureFilter::NearestMipmapNearest:  return GL_NEAREST_MIPMAP_NEAREST;
		case TextureFilter::LinearMipmapNearest:   return GL_LINEAR_MIPMAP_NEAREST;
		case TextureFilter::NearestMipmapLinear:   return GL_NEAREST_MIPMAP_LINEAR;
		case TextureFilter::LinearMipmapLinear:    return GL_LINEAR_MIPMAP_LINEAR;
		default:
			Log::CoreWarn("Unknown minification filter ({}). Falling back to GL_LINEAR.", static_cast<int>(filter));
			return GL_LINEAR;
		}
	}

	GLenum ToGLMagFilter(TextureFilter filter)
	{
		switch (filter)
		{
		case TextureFilter::Nearest: return GL_NEAREST;
		case TextureFilter::Linear:  return GL_LINEAR;
		default:
			if (filter == TextureFilter::NearestMipmapNearest || filter == TextureFilter::NearestMipmapLinear)
			{
				Log::CoreWarn("Magnification filter does not support mipmap-based option ({}). Using GL_NEAREST.", static_cast<int>(filter));
				return GL_NEAREST;
			}

			Log::CoreWarn("Magnification filter does not support mipmap-based option ({}). Using GL_LINEAR.", static_cast<int>(filter));
			return GL_LINEAR;
		}
	}

	GLenum ToGLInternalFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::R8:      return GL_R8;
		case ImageFormat::RGB8:    return GL_RGB8;
		case ImageFormat::RGBA8:   return GL_RGBA8;
		case ImageFormat::R32F:    return GL_R32F;
		case ImageFormat::RGB32F:  return GL_RGB32F;
		case ImageFormat::RGBA32F: return GL_RGBA32F;
		default: return GL_RGBA8;
		}
	}

	static GLenum ToGLInternalFormat(TextureFormat fmt)
	{
		switch (fmt)
		{
		case TextureFormat::RGBA8:           return GL_RGBA8;
		case TextureFormat::RGBA8_sRGB:      return GL_SRGB8_ALPHA8;
		case TextureFormat::RGBA16F:         return GL_RGBA16F;
		case TextureFormat::RGBA32F:         return GL_RGBA32F;

		case TextureFormat::Depth24:         return GL_DEPTH_COMPONENT24;
		case TextureFormat::Depth32F:        return GL_DEPTH_COMPONENT32F;
		case TextureFormat::Depth24Stencil8: return GL_DEPTH24_STENCIL8;

		default:                             return GL_RGBA8;
		}
	}

	GLenum ToGLImageFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::R8:
		case ImageFormat::R32F:    return GL_RED;
		case ImageFormat::RGB8:
		case ImageFormat::RGB32F:  return GL_RGB;
		case ImageFormat::RGBA8:
		case ImageFormat::RGBA32F: return GL_RGBA;
		default:
			Log::CoreWarn("Unknown base ImageFormat ({}). Falling back to GL_RGBA.", static_cast<int>(format));
			return GL_RGBA;
		}
	}

	GLenum ToGLDataType(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGB32F:
		case ImageFormat::RGBA32F:
		case ImageFormat::R32F:
			return GL_FLOAT;

		case ImageFormat::R8:
		default:
			return GL_UNSIGNED_BYTE;
		}
	}

	void ApplyTextureParameters(const TextureSpecs& specs)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGLTextureWrap(specs.WrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGLTextureWrap(specs.WrapT));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ToGLMinFilter(specs.MinFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ToGLMagFilter(specs.MagFilter));
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

		const GLenum internal_format = ToGLInternalFormat(m_specs.Format);
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_specs.Width, m_specs.Height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void GLTexture2D::SetData(ImageFormat source_format, std::span<const std::byte> data) const
	{
		const GLenum internal_format = ToGLInternalFormat(m_specs.Format);
		const GLenum data_format = ToGLImageFormat(source_format);
		const GLenum data_type = ToGLDataType(source_format);

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
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, ToGLMinFilter(m_specs.MinFilter));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, ToGLMagFilter(m_specs.MagFilter));

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
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, ToGLMinFilter(m_specs.MinFilter));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, ToGLMagFilter(m_specs.MagFilter));

		const GLenum internal_format = ToGLInternalFormat(m_specs.Format);

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

		const GLenum internal_format = ToGLInternalFormat(m_specs.Format);
		const GLenum data_format = ToGLImageFormat(source_format);
		const GLenum data_type = ToGLDataType(source_format);

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