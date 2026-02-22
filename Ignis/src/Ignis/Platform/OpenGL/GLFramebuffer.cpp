#include "GLFramebuffer.h"
#include "GLTexture.h"
#include "Ignis/Core/Log.h"

namespace ignis {

	GLFramebuffer::GLFramebuffer(const FramebufferSpec& spec)
		: m_spec(spec)
	{
		Log::CoreInfo("GLFramebuffer created (placeholder - Phase 2 will implement)");
	}

	GLFramebuffer::~GLFramebuffer()
	{
	}

	void GLFramebuffer::Invalidate()
	{
	}

	void GLFramebuffer::Release()
	{
	}

	void GLFramebuffer::Bind() const
	{
	}

	void GLFramebuffer::Unbind() const
	{
	}

	void GLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		
	}

	std::shared_ptr<Texture2D> GLFramebuffer::GetColorAttachment(uint32_t index) const
	{
		return nullptr;
	}

	uint32_t GLFramebuffer::GetColorAttachmentID(uint32_t index) const
	{
		return 0;
	}

} // namespace ignis
