#include "GLFramebuffer.h"
#include "GLTexture.h"
#include "Ignis/Core/Log.h"
#include <glad/glad.h>

namespace ignis {

	GLFramebuffer::GLFramebuffer(const FramebufferSpec& spec)
		: m_spec(spec)
	{
		Invalidate();
	}

	GLFramebuffer::~GLFramebuffer()
	{
		Release();
	}

	void GLFramebuffer::Invalidate()
	{
		// Clean up existing resources if resizing
		if (m_fbo_id)
		{
			Release();
		}

		// Create framebuffer
		glGenFramebuffers(1, &m_fbo_id);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_id);

		// Create color attachments
		if (m_spec.ColorAttachments.empty())
		{
			// Default: single RGBA8 color attachment
			m_spec.ColorAttachments.push_back({ImageFormat::RGBA8});
		}

		m_color_attachment_ids.resize(m_spec.ColorAttachments.size());
		m_color_attachments.resize(m_spec.ColorAttachments.size());

		for (size_t i = 0; i < m_spec.ColorAttachments.size(); i++)
		{
			const auto& attachment_spec = m_spec.ColorAttachments[i];
			
			// Create texture for color attachment
			TextureSpecs tex_spec;
			tex_spec.Width = m_spec.Width;
			tex_spec.Height = m_spec.Height;
			tex_spec.InternalFormat = attachment_spec.Format;
			tex_spec.SourceFormat = attachment_spec.Format;
			tex_spec.GenMipmaps = false;
			tex_spec.MinFilter = TextureFilter::Linear;
			tex_spec.MagFilter = TextureFilter::Linear;
			tex_spec.WrapS = TextureWrap::ClampToEdge;
			tex_spec.WrapT = TextureWrap::ClampToEdge;

			// Create empty texture (no data)
			std::vector<std::byte> empty_data(tex_spec.Width * tex_spec.Height * 4, std::byte{0});
			auto texture = Texture2D::Create(tex_spec, empty_data);
			m_color_attachments[i] = texture;
			
			// Get OpenGL texture ID
			auto gl_texture = std::static_pointer_cast<GLTexture2D>(texture);
			m_color_attachment_ids[i] = gl_texture->GetID();

			// Attach to framebuffer
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
				GL_TEXTURE_2D,
				m_color_attachment_ids[i],
				0
			);
		}

		// Create depth attachment (renderbuffer)
		if (m_spec.HasDepthAttachment)
		{
			glGenRenderbuffers(1, &m_depth_attachment_id);
			glBindRenderbuffer(GL_RENDERBUFFER, m_depth_attachment_id);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_spec.Width, m_spec.Height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_attachment_id);
		}

		// Specify which color attachments to use
		if (m_color_attachment_ids.size() > 1)
		{
			std::vector<GLenum> buffers(m_color_attachment_ids.size());
			for (size_t i = 0; i < buffers.size(); i++)
				buffers[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
			glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
		}
		else if (m_color_attachment_ids.empty())
		{
			glDrawBuffer(GL_NONE);
		}

		// Check framebuffer completeness
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			Log::CoreError("Framebuffer is incomplete! Status: 0x{:X}", status);
		}
		else
		{
			Log::CoreInfo("GLFramebuffer created successfully: {}x{} with {} color attachment(s)", 
				m_spec.Width, m_spec.Height, m_color_attachment_ids.size());
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GLFramebuffer::Release()
	{
		if (m_fbo_id)
		{
			glDeleteFramebuffers(1, &m_fbo_id);
			m_fbo_id = 0;
		}

		if (m_depth_attachment_id)
		{
			glDeleteRenderbuffers(1, &m_depth_attachment_id);
			m_depth_attachment_id = 0;
		}

		m_color_attachment_ids.clear();
		m_color_attachments.clear();
	}

	void GLFramebuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_id);
		glViewport(0, 0, m_spec.Width, m_spec.Height);
	}

	void GLFramebuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > 8192 || height > 8192)
		{
			Log::CoreWarn("Attempted to resize framebuffer to invalid size: {}x{}", width, height);
			return;
		}

		if (m_spec.Width == width && m_spec.Height == height)
		{
			return; // No change needed
		}

		m_spec.Width = width;
		m_spec.Height = height;
		Invalidate();
	}

	std::shared_ptr<Texture2D> GLFramebuffer::GetColorAttachment(uint32_t index) const
	{
		if (index >= m_color_attachments.size())
			return nullptr;
		return m_color_attachments[index];
	}

	uint32_t GLFramebuffer::GetColorAttachmentID(uint32_t index) const
	{
		if (index >= m_color_attachment_ids.size())
			return 0;
		return m_color_attachment_ids[index];
	}

} // namespace ignis
