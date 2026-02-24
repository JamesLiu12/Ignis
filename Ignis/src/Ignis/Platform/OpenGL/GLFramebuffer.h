#pragma once

#include "Ignis/Renderer/Framebuffer.h"

namespace ignis
{
	class GLFramebuffer : public Framebuffer
	{
	public:
		GLFramebuffer(const FrameBufferSpecs& specs);
		virtual ~GLFramebuffer();

		std::shared_ptr<Texture2D> GetColorAttachment(uint32_t index = 0) const override;
		std::shared_ptr<Texture2D> GetDepthAttachment() const override;

		uint32_t GetWidth() const { return m_specs.Width; }
		uint32_t GetHeight() const override { return m_specs.Height; }

		void Resize(uint32_t width, uint32_t height) override;
		void Invalidate() override;

		void Bind() const override;
		void UnBind() const override;

		bool IsComplete() const;

	private:
		void CreateAttachments();
		void DeleteAttachments();

	private:
		uint32_t m_id = 0;
		FrameBufferSpecs m_specs;

		std::vector<std::shared_ptr<Texture2D>> m_color_attachments;
		std::shared_ptr<Texture2D> m_depth_attachment;
	};
}