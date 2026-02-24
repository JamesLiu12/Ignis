#pragma once

#include "Ignis/Renderer/Texture.h"
#include <imgui.h>

namespace ignis
{
	// Platform-agnostic ImGui texture rendering helper
	// Encapsulates platform-specific texture ID conversion and UV coordinate handling
	class ImGuiTextureHelper
	{
	public:
		virtual ~ImGuiTextureHelper() = default;
		
		// Render texture in ImGui window
		// Handles platform-specific texture ID conversion and UV coordinates
		virtual void RenderTexture(
			const std::shared_ptr<Texture>& texture,
			const ImVec2& size
		) const = 0;
		
		// Factory method - creates platform-specific implementation
		static std::unique_ptr<ImGuiTextureHelper> Create();
	};
}
