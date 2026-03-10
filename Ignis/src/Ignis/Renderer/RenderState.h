#pragma once
#include "Ignis/Core/API.h"

namespace ignis
{
	struct IGNIS_API RenderState
	{
		bool  DepthTest = true;
		bool  DepthWrite = true;

		enum class DepthFunc { Less, LessOrEqual, Equal, Always, Never };
		DepthFunc Depth = DepthFunc::Less;

		bool CullFace = true;
		enum class CullMode { Back, Front, FrontAndBack };
		CullMode Cull = CullMode::Back;

		bool Blend = false;
		enum class BlendFactor
		{
			Zero, One,
			SrcAlpha, OneMinusSrcAlpha,
			SrcColor, OneMinusSrcColor,
		};
		BlendFactor BlendSrc = BlendFactor::SrcAlpha;
		BlendFactor BlendDst = BlendFactor::OneMinusSrcAlpha;

		static RenderState Default()
		{
			return {};
		}

		static RenderState Overlay()
		{
			RenderState s;
			s.DepthTest = false;
			s.DepthWrite = false;
			s.CullFace = false;
			return s;
		}

		static RenderState Transparent()
		{
			RenderState s;
			s.DepthTest = true;
			s.DepthWrite = false;
			s.CullFace = false;
			s.Blend = true;
			return s;
		}

		static RenderState Skybox()
		{
			RenderState s;
			s.Depth = DepthFunc::LessOrEqual;
			s.CullFace = false;
			return s;
		}
	};
}