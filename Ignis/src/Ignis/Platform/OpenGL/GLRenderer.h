#pragma once

#include "Ignis/Renderer/Renderer.h"

namespace ignis
{
	struct LightEnvironment;
	struct Environment;

	class GLRenderer : public Renderer
	{
	public:
		GLRenderer();
		~GLRenderer() override = default;

		void BeginScene(std::shared_ptr<Pipeline> pipeline, std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera) override;
		void EndScene() override;

		void SetClearColor(float r, float g, float b, float a) override;
		void SetClearColor(const glm::vec4& color) override;

		void SetViewport(int x, int y, int width, int height) override;
		void SetViewport(const glm::ivec4& viewport) override;

		void DrawIndexed(VertexArray& va) override;
		void RenderMesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& model) override;
		void Clear() override;

		void SetFramebuffer(std::shared_ptr<Framebuffer> framebuffer) { m_framebuffer = framebuffer; }
		std::shared_ptr<Framebuffer> GetFramebuffer() const { return m_framebuffer; }

	private:
		LightEnvironment m_light_environment;
		Environment m_scene_environment;
		EnvironmentSettings m_environment_settings;
		std::shared_ptr<Pipeline> m_pipeline;
		std::shared_ptr<Scene> m_scene;
		std::shared_ptr<Camera> m_camera;
		std::shared_ptr<VertexArray> m_skybox_vao;

		std::shared_ptr<Framebuffer> m_framebuffer;
		std::shared_ptr<VertexArray> m_quad_vao;
		std::shared_ptr<Shader> m_screen_shader;
	};
}