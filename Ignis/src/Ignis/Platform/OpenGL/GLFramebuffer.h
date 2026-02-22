#pragma once

#include "Ignis/Renderer/Framebuffer.h"

namespace ignis {

	class GLFramebuffer : public Framebuffer
	{
	public:
		GLFramebuffer(const FramebufferSpec& spec);
		~GLFramebuffer() override;

		void Bind() const override;
		void Unbind() const override;
		void Resize(uint32_t width, uint32_t height) override;

		uint32_t GetWidth() const override { return m_spec.Width; }
		uint32_t GetHeight() const override { return m_spec.Height; }
		
		std::shared_ptr<Texture2D> GetColorAttachment(uint32_t index = 0) const override;
		uint32_t GetColorAttachmentID(uint32_t index = 0) const override;
		
		const FramebufferSpec& GetSpecification() const override { return m_spec; }

	private:
		void Invalidate(); // Create/recreate FBO
		void Release();    // Clean up OpenGL resources

	private:
		FramebufferSpec m_spec;
		uint32_t m_fbo_id = 0;
		
		// Color attachments (textures)
		std::vector<uint32_t> m_color_attachment_ids;
		std::vector<std::shared_ptr<Texture2D>> m_color_attachments;
		
		// Depth attachment (renderbuffer)
		uint32_t m_depth_attachment_id = 0;
	};

} // namespace ignis
