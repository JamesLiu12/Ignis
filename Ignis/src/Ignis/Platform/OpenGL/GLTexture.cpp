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

	GLenum ToGLImageFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::R8:      return GL_RED;
		case ImageFormat::RGB8:    return GL_RGB;
		case ImageFormat::RGBA8:   return GL_RGBA;
		case ImageFormat::RGBA32F: return GL_RGBA;
		default:
			Log::CoreWarn("Unknown base ImageFormat ({}). Falling back to GL_RGBA.", static_cast<int>(format));
			return 0;
		}
	}

	void ApplyTextureParameters(const TextureSpecs& specs)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGLTextureWrap(specs.WrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGLTextureWrap(specs.WrapT));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ToGLMinFilter(specs.MinFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ToGLMagFilter(specs.MagFilter));
	}

	void AllocateTextureStorage(const TextureSpecs& specs, const void* pixels)
	{
		const GLenum internal_format = ToGLImageFormat(specs.InternalFormat);
		const GLenum data_format = ToGLImageFormat(specs.SourceFormat);

		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, specs.Width, specs.Height, 0, data_format, GL_UNSIGNED_BYTE, pixels);

		if (specs.GenMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	GLTexture2D::GLTexture2D(const TextureSpecs& specs, std::span<const std::byte> data)
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

		AllocateTextureStorage(m_specs, static_cast<const void*>(data.data()));

		glBindTexture(GL_TEXTURE_2D, 0);
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

}