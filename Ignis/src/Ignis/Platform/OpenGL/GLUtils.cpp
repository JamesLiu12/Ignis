#include "GLUtils.h"

namespace ignis::utils
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

	GLenum ToGLTextureFormat(TextureFormat fmt)
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
}