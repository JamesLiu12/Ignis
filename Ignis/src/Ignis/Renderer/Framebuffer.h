#pragma once

#include "Texture.h"

namespace ignis
{
	struct FramebufferAttachmentSpecs
	{

		FramebufferAttachmentSpecs(TextureFormat format)
			: Format(format) {}

		TextureFormat Format;
	};

	struct FrameBufferSpecs
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint32_t Samples = 1;
		std::vector<FramebufferAttachmentSpecs> Attachments;
	};

	class Framebuffer
	{
	public:
		virtual std::shared_ptr<Texture2D> GetColorAttachment(uint32_t index = 0) const = 0;
		virtual std::shared_ptr<Texture2D> GetDepthAttachment() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Invalidate() = 0;

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		static std::shared_ptr<Framebuffer> Create(const FrameBufferSpecs& specs);
	};
}