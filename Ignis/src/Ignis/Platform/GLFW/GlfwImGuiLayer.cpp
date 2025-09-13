#include "pch.h"
#include "GlfwImGuiLayer.h"
#include "Ignis/Core/Application.h"

// ImGui headers
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// OpenGL headers
#ifdef __APPLE__
    #define GL_GLEXT_PROTOTYPES
    #define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

namespace ignis {

	GlfwImGuiLayer::GlfwImGuiLayer()
	{
	}

	GlfwImGuiLayer::~GlfwImGuiLayer()
	{
	}

	void GlfwImGuiLayer::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		SetDarkTheme();

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void GlfwImGuiLayer::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void GlfwImGuiLayer::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void GlfwImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void GlfwImGuiLayer::OnEvent(EventBase& e)
	{
		if (m_block_events)
		{
			ImGuiIO& io = ImGui::GetIO();
			// Simplified event blocking - just check if ImGui wants to capture input
			if (io.WantCaptureMouse || io.WantCaptureKeyboard)
			{
				e.Handled = true;
			}
		}
	}

}
