#include "GLImGuiTextureHelper.h"
#include "Ignis/Platform/OpenGL/GLTexture.h"
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
		
		// Cast to OpenGL-specific texture type to access OpenGL texture ID
		auto gl_texture = std::static_pointer_cast<GLTexture2D>(texture);
		uint32_t texture_id = gl_texture->GetOpenGLTextureID();
		ImTextureID imgui_texture_id = (ImTextureID)(intptr_t)texture_id;
		
		// OpenGL-specific: UV coordinates (Y-flipped for framebuffer textures)
		// Top-left: (0, 1), Bottom-right: (1, 0)
		ImVec2 uv0(0.0f, 1.0f);
		ImVec2 uv1(1.0f, 0.0f);
		
		ImGui::Image(imgui_texture_id, size, uv0, uv1);
	}
}
