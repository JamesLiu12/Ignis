#pragma once

#include "Ignis/Renderer/Renderer.h"

namespace ignis
{
	struct LightEnvironment;
	struct Environment;

	class GLRenderer : public Renderer
	{
	public:
		GLRenderer() = default;
		~GLRenderer() override = default;

		void BeginScene(Scene& scene, const Camera& camera) override;
		void EndScene() override;

		void SetClearColor(float r, float g, float b, float a) override;
		void SetClearColor(const glm::vec4& color) override;

		void SetViewport(int x, int y, int width, int height) override;
		void SetViewport(const glm::ivec4& viewport) override;

		void DrawIndexed(VertexArray& va) override;
		void RenderMesh(const std::shared_ptr<Pipeline> pipeline, const Camera& camera, const std::shared_ptr<Mesh>& mesh, const glm::mat4& model) override;
		void Clear() override;

	private:
		LightEnvironment m_light_environment;
		Environment m_scene_environment;
		EnvironmentSettings m_environment_settings;
	};
}