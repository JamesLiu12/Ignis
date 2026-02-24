#include "ImGuiTextureHelper.h"
#include "Ignis/Renderer/GraphicsAPI.h"
#include "Ignis/Core/Log.h"

// Platform-specific includes
#include "Ignis/Platform/OpenGL/ImGui/GLImGuiTextureHelper.h"

namespace ignis
{
	std::unique_ptr<ImGuiTextureHelper> ImGuiTextureHelper::Create()
	{
		switch (GraphicsAPI::GetType())
		{
			case GraphicsAPI::Type::OpenGL:
				return std::make_unique<GLImGuiTextureHelper>();
			
			// Future extension
			// case GraphicsAPI::Type::Vulkan:
			//     return std::make_unique<VulkanImGuiTextureHelper>();
		}
		
		Log::CoreError("Unknown graphics API type");
		return nullptr;
	}
}
