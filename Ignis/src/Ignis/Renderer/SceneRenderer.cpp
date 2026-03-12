#include "SceneRenderer.h"

namespace ignis
{
	SceneRenderer::SceneRenderer(Renderer& renderer)
		: m_renderer(renderer)
	{

	}

	SceneRenderer::~SceneRenderer()
	{

	}

	void SceneRenderer::BeginScene(const SceneRenderContext& context)
	{
		m_context = context;

		m_renderer.SetPipeline(context.Pipeline);
		m_renderer.SetCamera(context.Camera);


		if (m_context.Viewport.z > 0 && m_context.Viewport.w > 0)
		{
			m_renderer.SetViewport(m_context.Viewport);
		}

		if (m_context.ClearTarget)
		{
			m_renderer.SetClearColor(m_context.ClearColor);
			m_renderer.Clear();
		}
	}

	void SceneRenderer::EndScene()
	{
	}

	void SceneRenderer::SubmitMesh(const Mesh& mesh, const glm::mat4& transform) const
	{
		// Use scene environment if available, otherwise create a default empty one
		if (m_context.Scene->m_scene_environment)
		{
			m_renderer.RenderMesh(mesh, transform, *m_context.Scene->m_scene_environment, 
				m_context.Scene->m_environment_settings, m_context.Scene->m_light_environment);
		}
		else
		{
			// Create temporary empty environment for rendering without skybox
			Environment empty_env;
			m_renderer.RenderMesh(mesh, transform, empty_env, 
				m_context.Scene->m_environment_settings, m_context.Scene->m_light_environment);
		}
	}

	void SceneRenderer::SubmitSkybox() const
	{
		// Check if scene environment exists before accessing
		if (m_context.Scene->m_scene_environment && 
		    m_context.Scene->m_scene_environment->GetSkyboxMap())
		{
			m_renderer.RenderSkybox(*m_context.Scene->m_scene_environment, m_context.Scene->m_environment_settings);
		}
	}

	void SceneRenderer::SubmitText(const Font& font, const std::string& text, const glm::mat4& transform, const glm::vec4& color, float scale) const
	{
		m_renderer.RenderText(font, text, transform, color, scale);
	}
}