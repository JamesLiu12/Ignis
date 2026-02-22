#pragma once

#include "Texture.h"
#include "Image.h"
#include <glm/glm.hpp>

namespace ignis {

	struct FramebufferAttachmentSpec
	{
		ImageFormat Format = ImageFormat::RGBA8;
	};

	struct FramebufferSpec
	{
		uint32_t Width = 1280;
		uint32_t Height = 720;
		glm::vec4 ClearColor = {0.1f, 0.1f, 0.1f, 1.0f};
		
		// Attachments
		std::vector<FramebufferAttachmentSpec> ColorAttachments;
		FramebufferAttachmentSpec DepthAttachment;
		
		bool HasDepthAttachment = true;
		uint32_t Samples = 1; // For MSAA (start with 1 = no multisampling)
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		
		// Get color attachment texture for ImGui::Image()
		virtual std::shared_ptr<Texture2D> GetColorAttachment(uint32_t index = 0) const = 0;
		virtual uint32_t GetColorAttachmentID(uint32_t index = 0) const = 0;
		
		virtual const FramebufferSpec& GetSpecification() const = 0;

		static std::shared_ptr<Framebuffer> Create(const FramebufferSpec& spec);
	};

} // namespace ignis
