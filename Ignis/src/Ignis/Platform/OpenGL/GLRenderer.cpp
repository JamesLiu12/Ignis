#include "GLRenderer.h"
#include "Ignis/Renderer/Shader.h"

#include <glad/glad.h>

namespace ignis
{
	namespace
	{
		static constexpr float kCubeVertices[] = {
			-1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
		};

		static constexpr float kQuadVertices[] = {
			// pos      // uv
			-1.0f,  1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			 1.0f, -1.0f, 1.0f, 0.0f,

			-1.0f,  1.0f, 0.0f, 1.0f,
			 1.0f, -1.0f, 1.0f, 0.0f,
			 1.0f,  1.0f, 1.0f, 1.0f
		};

		static constexpr uint32_t kMaxTextQuads = 4096;
		static constexpr uint32_t kMaxTextVertices = kMaxTextQuads * 4;

		static GLenum ToGL(RenderState::DepthFunc f)
		{
			switch (f)
			{
			case RenderState::DepthFunc::Less:        return GL_LESS;
			case RenderState::DepthFunc::LessOrEqual: return GL_LEQUAL;
			case RenderState::DepthFunc::Equal:       return GL_EQUAL;
			case RenderState::DepthFunc::Always:      return GL_ALWAYS;
			case RenderState::DepthFunc::Never:       return GL_NEVER;
			}
			return GL_LESS;
		}

		static GLenum ToGL(RenderState::BlendFactor f)
		{
			switch (f)
			{
			case RenderState::BlendFactor::Zero:             return GL_ZERO;
			case RenderState::BlendFactor::One:              return GL_ONE;
			case RenderState::BlendFactor::SrcAlpha:         return GL_SRC_ALPHA;
			case RenderState::BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
			case RenderState::BlendFactor::SrcColor:         return GL_SRC_COLOR;
			case RenderState::BlendFactor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
			}
			return GL_ONE;
		}
	}

	GLRenderer::GLRenderer()
	{

	}

	void GLRenderer::Init()
	{
		m_shader_library = std::make_unique<ShaderLibrary>();

		m_shader_library->Load("resources://shaders/IgnisPBR.glsl", "IgnisPBR");
		m_shader_library->Load("resources://shaders/Skybox.glsl", "Skybox");
		m_shader_library->Load("resources://shaders/EquirectToCube.glsl", "EquirectToCube");
		m_shader_library->Load("resources://shaders/IrradianceConvolution.glsl", "IrradianceConvolution");
		m_shader_library->Load("resources://shaders/PrefilterGGX.glsl", "PrefilterGGX");
		m_shader_library->Load("resources://shaders/BRDFIntegration.glsl", "BRDFIntegration");
		m_shader_library->Load("resources://shaders/Text.glsl", "Text");
		m_shader_library->Load("resources://shaders/UI.glsl", "UI");

		// Cube
		m_cube_vao = VertexArray::Create();
		auto cube_vbo = VertexBuffer::Create(kCubeVertices, sizeof(kCubeVertices));
		cube_vbo->SetLayout({ { 0, Shader::DataType::Float3 } });
		m_cube_vao->AddVertexBuffer(cube_vbo);

		// Quad
		m_quad_vao = VertexArray::Create();
		auto quad_vbo = VertexBuffer::Create(kQuadVertices, sizeof(kQuadVertices));
		quad_vbo->SetLayout({
			{ 0, Shader::DataType::Float2 },
			{ 1, Shader::DataType::Float2 }
			});
		m_quad_vao->AddVertexBuffer(quad_vbo);

		m_shader_library->Load("resources://screen.glsl");

		// Text
		m_text_vbo = VertexBuffer::Create(kMaxTextVertices * sizeof(float) * 4, VertexBuffer::Usage::Dynamic);
		m_text_vbo->SetLayout({
			{ 0, Shader::DataType::Float2 },   // a_Position
			{ 1, Shader::DataType::Float2 }    // a_TexCoord
		});

		m_text_vao = VertexArray::Create();
		m_text_vao->AddVertexBuffer(m_text_vbo);

		// Sprite quad (dynamic, reused every draw call)
		m_sprite_vbo = VertexBuffer::Create(4 * sizeof(float) * 4, VertexBuffer::Usage::Dynamic);
		m_sprite_vbo->SetLayout({
			{ 0, Shader::DataType::Float2 },  // a_Position
			{ 1, Shader::DataType::Float2 }   // a_TexCoord
			});
		m_sprite_vao = VertexArray::Create();
		m_sprite_vao->AddVertexBuffer(m_sprite_vbo);

		// Static index buffer shared by all sprite draws
		uint32_t sprite_indices[6] = { 0, 1, 2, 0, 2, 3 };
		auto sprite_ibo = IndexBuffer::Create(sprite_indices, sizeof(sprite_indices));
		m_sprite_vao->SetIndexBuffer(sprite_ibo);
	}

	void GLRenderer::BeginFrame()
	{
		if (m_framebuffer)
			m_framebuffer->Bind();

		glEnable(GL_DEPTH_TEST);
	}

	void GLRenderer::EndFrame()
	{
		if (m_framebuffer)
		{
			m_framebuffer->UnBind();
			// ImGui expects a clean buffer to render correctly
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}

	void GLRenderer::SetClearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void GLRenderer::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void GLRenderer::SetViewport(int x, int y, int width, int height)
	{
		m_viewport_width = width;
		m_viewport_height = height;
		glViewport(x, y, width, height);
	}

	void GLRenderer::SetViewport(const glm::ivec4& viewport)
	{
		m_viewport_width = viewport[2];
		m_viewport_height = viewport[3];
		glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
	}

	void GLRenderer::DrawIndexed(VertexArray& va)
	{
		va.Bind();
		glDrawElements(GL_TRIANGLES, va.GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		va.UnBind();
	}

	void GLRenderer::DrawLines(VertexArray& va, uint32_t vertex_count)
	{
		va.Bind();
		glDrawArrays(GL_LINES, 0, vertex_count);
		va.UnBind();
	}

	void GLRenderer::RenderMesh(const Mesh& mesh, const glm::mat4& model, 
		const Environment& scene_environment, const EnvironmentSettings& environment_settings, const LightEnvironment& light_environment)
	{
		auto vao = mesh.GetVertexArray();
		vao->Bind();

		const auto& materials_data = mesh.GetMaterialsData();

		for (const auto& sm : mesh.GetSubmeshes())
		{
			const auto& material_data = materials_data[sm.MaterialIndex];

			// TODO a performance bottleneck?
			auto material = m_pipeline->CreateMaterial(material_data);
			m_pipeline->ApplyEnvironment(*material, scene_environment, environment_settings, light_environment);

			material->Set("view", m_camera->GetView());
			material->Set("projection", m_camera->GetProjection());

			// TODO hard coded to be refactored
			material->Set("model", model);
			material->Set("viewPos", m_camera->GetPosition());

			if (material_data.DoubleSided)
				glDisable(GL_CULL_FACE);
			else
				glEnable(GL_CULL_FACE);

			material->Bind();
			glDrawElements(
				GL_TRIANGLES,
				sm.IndexCount,
				GL_UNSIGNED_INT,
				(void*)(sm.BaseIndex * sizeof(uint32_t))
			);
		}

		glEnable(GL_CULL_FACE);
		vao->UnBind();
	}

	void GLRenderer::RenderSkybox(const Environment& environment, const EnvironmentSettings& environment_settings)
	{
		SetRenderState(RenderState::Skybox());

		auto material = m_pipeline->CreateSkyboxMaterial(environment, environment_settings);

		glm::mat4 view = glm::mat4(glm::mat3(m_camera->GetView()));
		material->Set("view", view);
		material->Set("projection", m_camera->GetProjection());

		material->Bind();
		RenderCube();

		ResetRenderState();
	}

	void GLRenderer::RenderText(const Font& font, const std::string& text, const glm::mat4& transform, const glm::vec4& color, float scale)
	{
		if (text.empty() || !font.GetAtlas())
			return;

		struct Vertex { float x, y, u, v; };
		std::vector<Vertex>   vertices;
		std::vector<uint32_t> indices;
		vertices.reserve(text.size() * 4);
		indices.reserve(text.size() * 6);

		float    cursor_x = 0.0f;
		float    cursor_y = 0.0f;
		uint32_t quad_idx = 0;

		for (unsigned char c : text)
		{
			if (c == '\n')
			{
				cursor_x = 0.0f;
				cursor_y += font.GetLineHeight() * scale;
				continue;
			}

			const GlyphMetrics* g = font.GetGlyph(static_cast<char>(c));
			if (!g)
			{
				if (const GlyphMetrics* sp = font.GetGlyph(' '))
					cursor_x += sp->Advance * scale;
				continue;
			}

			const float px0 = cursor_x + g->QuadMin.x * scale;
			const float py0 = cursor_y - g->QuadMax.y * scale;
			const float px1 = cursor_x + g->QuadMax.x * scale;
			const float py1 = cursor_y - g->QuadMin.y * scale;

			const float u0 = g->AtlasMin.x, v0 = g->AtlasMin.y;
			const float u1 = g->AtlasMax.x, v1 = g->AtlasMax.y;

			vertices.push_back({ px0, py1, u0, v0 });
			vertices.push_back({ px1, py1, u1, v0 });
			vertices.push_back({ px1, py0, u1, v1 });
			vertices.push_back({ px0, py0, u0, v1 });

			const uint32_t b = quad_idx * 4;
			indices.insert(indices.end(), { b, b + 1, b + 2,  b, b + 2, b + 3 });

			cursor_x += g->Advance * scale;
			++quad_idx;
		}

		if (vertices.empty())
			return;

		m_text_vbo->SetData(vertices.data(), vertices.size() * sizeof(Vertex));

		auto ibo = IndexBuffer::Create(
			indices.data(),
			static_cast<uint32_t>(indices.size() * sizeof(uint32_t)));
		m_text_vao->SetIndexBuffer(ibo);

		const glm::vec2 screen_size = { m_viewport_width, m_viewport_height };

		auto mat = Material::Create(m_shader_library->Get("Text"));
		mat->Set("u_Model", transform);
		mat->Set("u_View", m_camera->GetView());
		mat->Set("u_Projection", m_camera->GetProjection());
		mat->Set("u_ScreenSize", screen_size);
		mat->Set("u_Color", color);
		mat->Set("u_Atlas", font.GetAtlas());
		font.GetAtlas()->Bind(0);

		SetRenderState(RenderState::Transparent());

		mat->Bind();
		DrawIndexed(*m_text_vao);

		ResetRenderState();
	}

	void GLRenderer::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void GLRenderer::SetFramebuffer(std::shared_ptr<Framebuffer> framebuffer)
	{
		m_framebuffer = framebuffer;
	}

	void GLRenderer::RenderCube()
	{
		m_cube_vao->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		m_cube_vao->UnBind();
	}

	void GLRenderer::RenderQuad()
	{
		m_quad_vao->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		m_quad_vao->UnBind();
	}

	void GLRenderer::RenderSprite(const glm::vec2& min, const glm::vec2& max)
	{
		// (0,0) top-left, UV (0,0)-(1,1)
		struct Vertex { float x, y, u, v; };
		Vertex verts[4] = {
			{ min.x, min.y, 0.0f, 0.0f },   // top-left
			{ max.x, min.y, 1.0f, 0.0f },   // top-right
			{ max.x, max.y, 1.0f, 1.0f },   // bottom-right
			{ min.x, max.y, 0.0f, 1.0f }    // bottom-left
		};
		m_sprite_vbo->SetData(verts, sizeof(verts));

		SetRenderState(RenderState::Transparent());

		DrawIndexed(*m_sprite_vao);

		ResetRenderState();
	}

	void GLRenderer::RenderUIText(const Font& font, const std::string& text,
		const glm::mat4& projection,
		const glm::mat4& model,
		const glm::vec4& color,
		float scale)
	{
		// Identical vertex-building logic to RenderText,
		// but uses explicit projection + identity view.
		if (text.empty() || !font.GetAtlas()) return;

		struct Vertex { float x, y, u, v; };
		std::vector<Vertex>   vertices;
		std::vector<uint32_t> indices;
		vertices.reserve(text.size() * 4);
		indices.reserve(text.size() * 6);

		float    cursor_x = 0.0f;
		float    cursor_y = 0.0f;
		uint32_t quad_idx = 0;

		for (unsigned char c : text)
		{
			if (c == '\n')
			{
				cursor_x = 0.0f;
				cursor_y -= font.GetLineHeight() * scale; // y-up: move down = subtract
				continue;
			}

			const GlyphMetrics* g = font.GetGlyph(static_cast<char>(c));
			if (!g)
			{
				if (const GlyphMetrics* sp = font.GetGlyph(' '))
					cursor_x += sp->Advance * scale;
				continue;
			}

			const float px0 = cursor_x + g->QuadMin.x * scale;
			const float py0 = cursor_y - g->QuadMax.y * scale;
			const float px1 = cursor_x + g->QuadMax.x * scale;
			const float py1 = cursor_y - g->QuadMin.y * scale;

			const float u0 = g->AtlasMin.x, v0 = g->AtlasMin.y;
			const float u1 = g->AtlasMax.x, v1 = g->AtlasMax.y;

			vertices.push_back({ px0, py1, u0, v0 });
			vertices.push_back({ px1, py1, u1, v0 });
			vertices.push_back({ px1, py0, u1, v1 });
			vertices.push_back({ px0, py0, u0, v1 });

			const uint32_t b = quad_idx * 4;
			indices.insert(indices.end(), { b, b + 1, b + 2,  b, b + 2, b + 3 });

			cursor_x += g->Advance * scale;
			++quad_idx;
		}

		if (vertices.empty()) return;

		m_text_vbo->SetData(vertices.data(), vertices.size() * sizeof(Vertex));
		auto ibo = IndexBuffer::Create(indices.data(),
			static_cast<uint32_t>(indices.size() * sizeof(uint32_t)));
		m_text_vao->SetIndexBuffer(ibo);

		auto mat = Material::Create(m_shader_library->Get("Text"));
		mat->Set("u_Model", model);
		mat->Set("u_View", glm::mat4(1.0f));   // identity ¡ª no 3D camera
		mat->Set("u_Projection", projection);
		mat->Set("u_ScreenSize", glm::vec2(static_cast<float>(m_viewport_width),
			static_cast<float>(m_viewport_height)));
		mat->Set("u_Color", color);
		mat->Set("u_Atlas", font.GetAtlas());
		font.GetAtlas()->Bind(0);

		SetRenderState(RenderState::Transparent());

		mat->Bind();
		DrawIndexed(*m_text_vao);

		ResetRenderState();
	}

	void GLRenderer::SetRenderState(const RenderState& state)
	{
		// Depth test
		state.DepthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		glDepthFunc(ToGL(state.Depth));
		glDepthMask(state.DepthWrite ? GL_TRUE : GL_FALSE);

		// Culling
		state.CullFace ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);

		// Blend
		if (state.Blend)
		{
			glEnable(GL_BLEND);
			glBlendFunc(ToGL(state.BlendSrc), ToGL(state.BlendDst));
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}

	void GLRenderer::ResetRenderState()
	{
		SetRenderState(RenderState::Default());
	}
}