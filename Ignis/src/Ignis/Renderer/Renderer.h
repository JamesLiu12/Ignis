#pragma once

#include <glm/glm.hpp>
#include "VertexArray.h"
#include "Camera.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "Framebuffer.h"

namespace ignis
{	
	class Renderer
	{
	public:
		virtual ~Renderer() = default;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		
		virtual void SetViewport(int x, int y, int width, int height) = 0;
		virtual void SetViewport(const glm::ivec4& viewport) = 0;

		virtual void DrawIndexed(VertexArray& va) = 0;
		virtual void RenderMesh(const Mesh& mesh, const glm::mat4& model,
			const Environment& scene_environment, const EnvironmentSettings& environment_settings, const LightEnvironment& light_environment) = 0;
		virtual void RenderSkybox(const Environment& environment) = 0;

		virtual void Clear() = 0;

		virtual void SetFramebuffer(std::shared_ptr<Framebuffer> framebuffer) = 0;
		virtual std::shared_ptr<Framebuffer> GetFramebuffer() const = 0;

		virtual void SetCamera(std::shared_ptr<Camera> camera) = 0;
		virtual void SetPipeline(std::shared_ptr<Pipeline> pipeline) = 0;

		static std::unique_ptr<Renderer> Create();

		static std::shared_ptr<Texture2D> GetWhiteTexture();
		static std::shared_ptr<Texture2D> GetBlackTexture();
		static std::shared_ptr<Texture2D> GetDefaultNormalTexture();
		static std::shared_ptr<Texture2D> GetDefaultRoughnessTexture();
		// Get asset handles for default textures (memory-only assets)
		static AssetHandle GetWhiteTextureHandle();
		static AssetHandle GetBlackTextureHandle();
		static AssetHandle GetDefaultNormalTextureHandle();
		static AssetHandle GetDefaultRoughnessTextureHandle();
	};
}