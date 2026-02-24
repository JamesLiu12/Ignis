#pragma once

#include <glad/glad.h>
#include "Ignis/Renderer/Texture.h"

namespace ignis::utils
{
	GLenum ToGLTextureWrap(TextureWrap wrap);
	GLenum ToGLMinFilter(TextureFilter filter);
	GLenum ToGLMagFilter(TextureFilter filter);
	GLenum ToGLTextureFormat(TextureFormat fmt);
	GLenum ToGLImageFormat(ImageFormat format);
	GLenum ToGLDataType(ImageFormat format);
}