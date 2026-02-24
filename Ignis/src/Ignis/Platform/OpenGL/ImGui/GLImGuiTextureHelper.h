#pragma once

#include "Ignis/ImGui/ImGuiTextureHelper.h"

namespace ignis
{
	// OpenGL-specific ImGui texture rendering helper
	// Handles OpenGL texture ID conversion and Y-flipped UV coordinates for framebuffers
	class GLImGuiTextureHelper : public ImGuiTextureHelper
	{
	public:
		GLImGuiTextureHelper() = default;
		~GLImGuiTextureHelper() override = default;
		
		void RenderTexture(
			const std::shared_ptr<Texture>& texture,
			const ImVec2& size
		) const override;
	};
}
