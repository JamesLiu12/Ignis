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

		m_renderer.BeginFrame();
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
		m_renderer.EndFrame();
	}

	void SceneRenderer::SubmitMesh(const Mesh& mesh, const glm::mat4& transform) const
	{
		m_renderer.RenderMesh(mesh, transform, m_context.Scene->m_scene_environment, 
			m_context.Scene->m_environment_settings, m_context.Scene->m_light_environment);
	}

	void SceneRenderer::SubmitSkybox() const
	{
		m_renderer.RenderSkybox(m_context.Scene->m_scene_environment);
	}
}