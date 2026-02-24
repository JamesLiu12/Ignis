#include "GLImGuiTextureHelper.h"
#include <imgui.h>

namespace ignis
{
	void GLImGuiTextureHelper::RenderTexture(
		const std::shared_ptr<Texture>& texture,
		const ImVec2& size
	) const
	{
		if (!texture)
			return;
		
		// OpenGL-specific: Get texture ID and cast to ImTextureID
		uint32_t texture_id = texture->GetRendererID();
		ImTextureID imgui_texture_id = (ImTextureID)(intptr_t)texture_id;
		
		// OpenGL-specific: UV coordinates (Y-flipped for framebuffer textures)
		// Top-left: (0, 1), Bottom-right: (1, 0)
		ImVec2 uv0(0.0f, 1.0f);
		ImVec2 uv1(1.0f, 0.0f);
		
		ImGui::Image(imgui_texture_id, size, uv0, uv1);
	}
}
