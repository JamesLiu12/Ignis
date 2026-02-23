#include "GLFramebuffer.h"
#include "GLTexture.h"
#include "Ignis/Renderer/Texture.h"

#include <glad/glad.h>

namespace ignis
{
	GLFramebuffer::GLFramebuffer(const FrameBufferSpecs& specs)
		: m_specs(specs)
	{
		Invalidate();
	}

	GLFramebuffer::~GLFramebuffer()
	{
		DeleteAttachments();
		
		if (m_id)
		{
			glDeleteFramebuffers(1, &m_id);
		}
	}

	void GLFramebuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
		glViewport(0, 0, m_specs.Width, m_specs.Height);
	}

	void GLFramebuffer::UnBind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
		{
			Log::CoreWarn("Invalid framebuffer size: {}x{}", width, height);
			return;
		}

		m_specs.Width = width;
		m_specs.Height = height;
		
		Invalidate();
	}

	void GLFramebuffer::Invalidate()
	{
		if (m_id)
		{
			DeleteAttachments();
			glDeleteFramebuffers(1, &m_id);
		}

		glGenFramebuffers(1, &m_id);
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);

		CreateAttachments();

		if (m_color_attachments.size() > 1)
		{
			std::vector<GLenum> buffers;
			for (uint32_t i = 0; i < m_color_attachments.size(); i++)
			{
				buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
			}
			glDrawBuffers((GLsizei)buffers.size(), buffers.data());
		}
		else if (m_color_attachments.empty())
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		if (!IsComplete())
		{
			Log::CoreError("Framebuffer is incomplete!");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GLFramebuffer::CreateAttachments()
	{
		m_color_attachments.clear();
		m_depth_attachment = nullptr;

		uint32_t color_attachment_index = 0;

		for (const auto& spec : m_specs.Attachments)
		{
			TextureSpecs tex_specs;
			tex_specs.Width = m_specs.Width;
			tex_specs.Height = m_specs.Height;
			tex_specs.Format = spec.Format;
			tex_specs.Samples = m_specs.Samples;
			tex_specs.GenMipmaps = false;
			tex_specs.MinFilter = TextureFilter::Linear;
			tex_specs.MagFilter = TextureFilter::Linear;
			tex_specs.WrapS = TextureWrap::ClampToEdge;
			tex_specs.WrapT = TextureWrap::ClampToEdge;

			switch (spec.Format)
			{
			case TextureFormat::RGBA8:
			case TextureFormat::RGBA8_sRGB:
			case TextureFormat::RGBA16F:
			case TextureFormat::RGBA32F:
			{
				tex_specs.Usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
				auto texture = Texture2D::Create(tex_specs);
				m_color_attachments.push_back(texture);
				
				glFramebufferTexture2D(
					GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + color_attachment_index,
					GL_TEXTURE_2D,
					std::static_pointer_cast<GLTexture2D>(texture)->m_id,
					0
				);
				
				color_attachment_index++;
				break;
			}
			
			case TextureFormat::Depth24:
			case TextureFormat::Depth32F:
			{
				tex_specs.Usage = TextureUsage::Depth | TextureUsage::Sampled;
				m_depth_attachment = Texture2D::Create(tex_specs);
				
				glFramebufferTexture2D(
					GL_FRAMEBUFFER,
					GL_DEPTH_ATTACHMENT,
					GL_TEXTURE_2D,
					std::static_pointer_cast<GLTexture2D>(m_depth_attachment)->m_id,
					0
				);
				break;
			}
			
			case TextureFormat::Depth24Stencil8:
			{
				tex_specs.Usage = TextureUsage::Depth | TextureUsage::Stencil | TextureUsage::Sampled;
				m_depth_attachment = Texture2D::Create(tex_specs);
				
				glFramebufferTexture2D(
					GL_FRAMEBUFFER,
					GL_DEPTH_STENCIL_ATTACHMENT,
					GL_TEXTURE_2D,
					std::static_pointer_cast<GLTexture2D>(m_depth_attachment)->m_id,
					0
				);
				break;
			}
			}
		}
	}

	void GLFramebuffer::DeleteAttachments()
	{
		m_color_attachments.clear();
		m_depth_attachment = nullptr;
	}

	std::shared_ptr<Texture2D> GLFramebuffer::GetColorAttachment(uint32_t index) const
	{
		if (index >= m_color_attachments.size())
			return nullptr;
		
		return m_color_attachments[index];
	}

	std::shared_ptr<Texture2D> GLFramebuffer::GetDepthAttachment() const
	{
		return m_depth_attachment;
	}

	bool GLFramebuffer::IsComplete() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		return status == GL_FRAMEBUFFER_COMPLETE;
	}
}