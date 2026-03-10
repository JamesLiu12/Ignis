#pragma once

#include "Ignis/Core/API.h"
#include "VertexArray.h"
#include "Camera.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "Framebuffer.h"
#include "ShaderLibrary.h"
#include "Font.h"
#include "RenderState.h"

#include <glm/glm.hpp>

namespace ignis
{	
	class IGNIS_API Renderer
	{
	public:
		virtual ~Renderer() = default;

		virtual void Init() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		
		virtual void SetViewport(int x, int y, int width, int height) = 0;
		virtual void SetViewport(const glm::ivec4& viewport) = 0;

		virtual void DrawIndexed(VertexArray& va) = 0;
		virtual void DrawLines(VertexArray& va, uint32_t vertex_count) = 0;

		virtual void RenderMesh(const Mesh& mesh, const glm::mat4& model,
			const Environment& scene_environment, const EnvironmentSettings& environment_settings, const LightEnvironment& light_environment) = 0;
		virtual void RenderSkybox(const Environment& environment) = 0;
		virtual void RenderText(const Font& font, const std::string& text, const glm::mat4& transform, const glm::vec4& color, float scale) = 0;

		virtual void Clear() = 0;

		virtual void SetFramebuffer(std::shared_ptr<Framebuffer> framebuffer) = 0;
		virtual std::shared_ptr<Framebuffer> GetFramebuffer() const = 0;

		virtual void SetCamera(std::shared_ptr<Camera> camera) = 0;
		virtual void SetPipeline(std::shared_ptr<Pipeline> pipeline) = 0;

		virtual const ShaderLibrary& GetShaderLibrary() const = 0;
		virtual ShaderLibrary& GetShaderLibrary() = 0;

		virtual void RenderCube() = 0;
		virtual void RenderQuad() = 0;

		virtual void RenderSprite(const glm::vec2& min, const glm::vec2& max) = 0;
		virtual void RenderUIText(const Font& font, const std::string& text, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color, float scale) = 0;

		virtual void SetRenderState(const RenderState& state) = 0;
		virtual void ResetRenderState() = 0;

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