#pragma once

#include "Renderer.h"
#include "Environment.h"
#include "Ignis/Scene/Scene.h"

namespace ignis
{
	class Scene;

	struct SceneRenderContext
	{
		std::shared_ptr<Scene> Scene;
		std::shared_ptr<Camera> Camera;
		std::shared_ptr<Pipeline> Pipeline;

		glm::ivec4 Viewport{ 0, 0, 0, 0 };
		glm::vec4 ClearColor{ 0.1f, 0.1f, 0.1f, 1.0f };
		bool ClearTarget = true;
	};

	class SceneRenderer
	{
	public:
		SceneRenderer(Renderer& renderer);
		~SceneRenderer();

		void BeginScene(const SceneRenderContext& context);
		void EndScene();

		void SubmitMesh(const Mesh& mesh, const glm::mat4& transform = glm::mat4(1.0f)) const;
		void SubmitSkybox() const;

	private:
		Renderer& m_renderer;
		SceneRenderContext m_context;

		LightEnvironment m_light_environment;
		Environment m_scene_environment;
		EnvironmentSettings m_environment_settings;
	};
}