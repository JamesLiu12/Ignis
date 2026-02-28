#include "GLIBLBaker.h"
#include "GLTexture.h"
#include "Ignis/Renderer/Shader.h"
#include "Ignis/Renderer/VertexArray.h"
#include "Ignis/Renderer/Renderer.h"
#include "Ignis/Renderer/Material.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

namespace ignis
{
	namespace
	{
		static glm::mat4 CaptureProjection()
		{
			return glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		}

		static std::array<glm::mat4, 6> CaptureViews()
		{
			return {
				glm::lookAt(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(0,-1, 0)),
				glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)),
				glm::lookAt(glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
				glm::lookAt(glm::vec3(0), glm::vec3(0,-1, 0), glm::vec3(0, 0,-1)),
				glm::lookAt(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(0,-1, 0)),
				glm::lookAt(glm::vec3(0), glm::vec3(0, 0,-1), glm::vec3(0,-1, 0))
			};
		}

		static uint32_t CalcMipLevels(uint32_t size)
		{
			return static_cast<uint32_t>(std::floor(std::log2((float)size))) + 1;
		}

		static bool CheckCaptureFBOComplete()
		{
			return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		}
	}

	IBLBakeResult GLIBLBaker::BakeFromEquirectangular(const Image& hdr_image, const IBLBakeSettings& settings) const
	{
		IBLBakeResult out{};

		GLint prev_viewport[4] = { 0, 0, 0, 0 };
		glGetIntegerv(GL_VIEWPORT, prev_viewport);
		const GLboolean cull_was_enabled = glIsEnabled(GL_CULL_FACE);

		glDisable(GL_CULL_FACE);

		// 1) Upload equirect HDR to 2D texture
		TextureSpecs hdr_specs{};
		hdr_specs.Width = hdr_image.GetWidth();
		hdr_specs.Height = hdr_image.GetHeight();
		hdr_specs.Format = TextureFormat::RGBA32F;
		hdr_specs.MinFilter = TextureFilter::Linear;
		hdr_specs.MagFilter = TextureFilter::Linear;
		hdr_specs.WrapS = TextureWrap::ClampToEdge;
		hdr_specs.WrapT = TextureWrap::ClampToEdge;
		hdr_specs.GenMipmaps = false;

		auto hdrTex = Texture2D::Create(hdr_specs, hdr_image.GetFormat(), hdr_image.GetPixels());
		if (!hdrTex)
		{
			if (cull_was_enabled) glEnable(GL_CULL_FACE);
			glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
			return out;
		}

		// 2) Create Environment Cube
		TextureSpecs env_specs{};
		env_specs.Width = settings.EnvironmentResolution;
		env_specs.Height = settings.EnvironmentResolution;
		env_specs.Format = TextureFormat::RGBA16F;
		env_specs.MinFilter = TextureFilter::LinearMipmapLinear;
		env_specs.MagFilter = TextureFilter::Linear;
		env_specs.WrapS = TextureWrap::ClampToEdge;
		env_specs.WrapT = TextureWrap::ClampToEdge;
		env_specs.GenMipmaps = true;
		out.EnvironmentCube = TextureCube::Create(env_specs);

		if (!out.EnvironmentCube)
		{
			if (cull_was_enabled) glEnable(GL_CULL_FACE);
			glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
			return out;
		}

		// 3) FBO/RBO
		GLuint capture_fbo = 0, capture_rbo = 0;
		glGenFramebuffers(1, &capture_fbo);
		glGenRenderbuffers(1, &capture_rbo);

		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, settings.EnvironmentResolution, settings.EnvironmentResolution);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);

		const GLuint env_cube_id = static_cast<GLTextureCube*>(out.EnvironmentCube.get())->m_id;

		auto eq_to_cube = Material::Create(m_renderer.GetShaderLibrary().Get("EquirectToCube"));
		eq_to_cube->GetShader()->Bind();
		eq_to_cube->Set("equirectangularMap", 0);
		eq_to_cube->Set("projection", CaptureProjection());

		hdrTex->Bind(0);

		auto views = CaptureViews();
		glViewport(0, 0, settings.EnvironmentResolution, settings.EnvironmentResolution);
		glClearColor(0.f, 0.f, 0.f, 1.f);

		for (uint32_t face = 0; face < 6; ++face)
		{
			eq_to_cube->Set("view", views[face]);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				env_cube_id, 0
			);

			if (!CheckCaptureFBOComplete())
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glDeleteFramebuffers(1, &capture_fbo);
				glDeleteRenderbuffers(1, &capture_rbo);
				if (cull_was_enabled) glEnable(GL_CULL_FACE);
				glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
				return out;
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_renderer.RenderCube();
		}

		// Create environment mip£¬for prefilter to sample
		glBindTexture(GL_TEXTURE_CUBE_MAP, env_cube_id);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// 4) Create Irradiance Cube
		TextureSpecs irrSpecs = env_specs;
		irrSpecs.Width = settings.IrradianceResolution;
		irrSpecs.Height = settings.IrradianceResolution;
		irrSpecs.Format = TextureFormat::RGBA16F;
		irrSpecs.MinFilter = TextureFilter::Linear;
		irrSpecs.MagFilter = TextureFilter::Linear;
		irrSpecs.GenMipmaps = false;
		out.IrradianceCube = TextureCube::Create(irrSpecs);

		if (!out.IrradianceCube)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &capture_fbo);
			glDeleteRenderbuffers(1, &capture_rbo);
			if (cull_was_enabled) glEnable(GL_CULL_FACE);
			glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
			return out;
		}

		const GLuint irr_cube_id = static_cast<GLTextureCube*>(out.IrradianceCube.get())->m_id;

		auto irradiance_shader = Material::Create(m_renderer.GetShaderLibrary().Get("IrradianceConvolution"));
		irradiance_shader->GetShader()->Bind();
		irradiance_shader->Set("environmentMap", 0);
		irradiance_shader->Set("projection", CaptureProjection());
		out.EnvironmentCube->Bind(0);

		glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, settings.IrradianceResolution, settings.IrradianceResolution);
		glViewport(0, 0, settings.IrradianceResolution, settings.IrradianceResolution);

		for (uint32_t face = 0; face < 6; ++face)
		{
			irradiance_shader->Set("view", views[face]);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				irr_cube_id, 0
			);

			if (!CheckCaptureFBOComplete())
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glDeleteFramebuffers(1, &capture_fbo);
				glDeleteRenderbuffers(1, &capture_rbo);
				if (cull_was_enabled) glEnable(GL_CULL_FACE);
				glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
				return out;
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_renderer.RenderCube();
		}

		// 5) Create Prefilter Cube (GGX importance sampling + mip chain)
		TextureSpecs pre_specs = env_specs;
		pre_specs.Width = settings.PrefilterResolution;
		pre_specs.Height = settings.PrefilterResolution;
		pre_specs.Format = TextureFormat::RGBA16F;
		pre_specs.MinFilter = TextureFilter::LinearMipmapLinear;
		pre_specs.MagFilter = TextureFilter::Linear;
		pre_specs.GenMipmaps = true;
		out.PrefilterCube = TextureCube::Create(pre_specs);
		out.PrefilterMipLevels = CalcMipLevels(settings.PrefilterResolution);

		if (!out.PrefilterCube)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &capture_fbo);
			glDeleteRenderbuffers(1, &capture_rbo);
			if (cull_was_enabled) glEnable(GL_CULL_FACE);
			glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
			return out;
		}

		const GLuint prefilter_cube_id = static_cast<GLTextureCube*>(out.PrefilterCube.get())->m_id;

		// Ensure mip chain has been allocated
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_cube_id);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		auto prefilter_shader = Material::Create(m_renderer.GetShaderLibrary().Get("PrefilterGGX"));
		prefilter_shader->GetShader()->Bind();
		prefilter_shader->Set("environmentMap", 0);
		prefilter_shader->Set("projection", CaptureProjection());
		out.EnvironmentCube->Bind(0);

		for (uint32_t mip = 0; mip < out.PrefilterMipLevels; ++mip)
		{
			uint32_t mip_w = std::max(1u, settings.PrefilterResolution >> mip);
			uint32_t mip_h = std::max(1u, settings.PrefilterResolution >> mip);

			glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_w, mip_h);
			glViewport(0, 0, mip_w, mip_h);

			float roughness = (out.PrefilterMipLevels <= 1)
				? 0.0f
				: (float)mip / (float)(out.PrefilterMipLevels - 1);
			prefilter_shader->Set("roughness", roughness);

			for (uint32_t face = 0; face < 6; ++face)
			{
				prefilter_shader->Set("view", views[face]);
				glFramebufferTexture2D(
					GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
					prefilter_cube_id, mip
				);

				if (!CheckCaptureFBOComplete())
				{
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glDeleteFramebuffers(1, &capture_fbo);
					glDeleteRenderbuffers(1, &capture_rbo);
					if (cull_was_enabled) glEnable(GL_CULL_FACE);
					glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
					return out;
				}

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				m_renderer.RenderCube();
			}
		}

		// 6) Create BRDF LUT 2D
		TextureSpecs brdf_specs{};
		brdf_specs.Width = settings.BrdfLUTResolution;
		brdf_specs.Height = settings.BrdfLUTResolution;
		brdf_specs.Format = TextureFormat::RG16F;
		brdf_specs.MinFilter = TextureFilter::Linear;
		brdf_specs.MagFilter = TextureFilter::Linear;
		brdf_specs.WrapS = TextureWrap::ClampToEdge;
		brdf_specs.WrapT = TextureWrap::ClampToEdge;
		brdf_specs.GenMipmaps = false;
		out.BrdfLUT = Texture2D::Create(brdf_specs);

		if (!out.BrdfLUT)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &capture_fbo);
			glDeleteRenderbuffers(1, &capture_rbo);
			if (cull_was_enabled) glEnable(GL_CULL_FACE);
			glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
			return out;
		}

		const GLuint brdflut_id = static_cast<GLTexture2D*>(out.BrdfLUT.get())->m_id;

		auto brdfShader = Material::Create(m_renderer.GetShaderLibrary().Get("BRDFIntegration"));
		brdfShader->GetShader()->Bind();

		glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, settings.BrdfLUTResolution, settings.BrdfLUTResolution);
		glViewport(0, 0, settings.BrdfLUTResolution, settings.BrdfLUTResolution);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdflut_id, 0);

		if (!CheckCaptureFBOComplete())
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &capture_fbo);
			glDeleteRenderbuffers(1, &capture_rbo);
			if (cull_was_enabled) glEnable(GL_CULL_FACE);
			glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
			return out;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_renderer.RenderQuad();

		// Clear and restore states
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &capture_fbo);
		glDeleteRenderbuffers(1, &capture_rbo);

		if (cull_was_enabled) glEnable(GL_CULL_FACE);
		glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);

		return out;
	}
}