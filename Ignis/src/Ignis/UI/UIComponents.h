#pragma once

#include "Ignis/Scene/Components.h"
#include "Ignis/Asset/Asset.h"

#include <glm/glm.hpp>
#include <string>

namespace ignis
{
	struct RectTransformComponent : Component
	{
		glm::vec2 AnchorMin = { 0.5f, 0.5f };
		glm::vec2 AnchorMax = { 0.5f, 0.5f };

		// Pixel offsets from anchor corners to rect corners
		glm::vec2 OffsetMin = { -50.0f, -50.0f };
		glm::vec2 OffsetMax = { 50.0f,  50.0f };

		// Written by UISystem::OnUpdate() - do NOT set manually
		glm::vec2 ResolvedMin = { 0.0f,   0.0f };
		glm::vec2 ResolvedMax = { 100.0f, 100.0f };

		glm::vec2 GetSize()   const { return ResolvedMax - ResolvedMin; }
		glm::vec2 GetCenter() const { return (ResolvedMin + ResolvedMax) * 0.5f; }
	};

	struct CanvasComponent : Component
	{
		enum class RenderMode { ScreenSpace, WorldSpace };

		RenderMode Mode = RenderMode::ScreenSpace;
		int        SortOrder = 0;   // higher = drawn on top
		bool       Visible = true;
	};

	struct ImageComponent : Component
	{
		enum class ScaleMode
		{
			Stretch,
			FitInside,
			FitOutside,
			NativeSize
		};

		AssetHandle Texture;                         // invalid = solid color
		glm::vec4   Color = glm::vec4(1.0f);
		bool        Visible = true;
		bool        RaycastTarget = true;            // participates in hit-testing
		ScaleMode Scale = ScaleMode::Stretch;
	};

	struct UITextComponent : Component
	{
		enum class HorizontalAlignment { Left, Center, Right };
		enum class VerticalAlignment { Top, Middle, Bottom };

		std::string          Text;
		AssetHandle          Font;
		glm::vec4            Color = glm::vec4(1.0f);
		float                FontSize = 16.0f;          // desired pixel height
		HorizontalAlignment  HAlign = HorizontalAlignment::Left;
		VerticalAlignment    VAlign = VerticalAlignment::Top;
		bool                 Visible = true;
	};

	struct ButtonComponent : Component
	{
		glm::vec4 NormalColor = { 1.00f, 1.00f, 1.00f, 1.0f };
		glm::vec4 HoverColor = { 0.85f, 0.85f, 0.85f, 1.0f };
		glm::vec4 PressedColor = { 0.65f, 0.65f, 0.65f, 1.0f };
		glm::vec4 DisabledColor = { 0.50f, 0.50f, 0.50f, 0.5f };
		bool      Interactable = true;

		// Runtime state - managed by UISystem, do NOT set manually
		bool      IsHovered = false;
		bool      IsPressed = false;
		glm::vec4 CurrentColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	};

	struct ProgressBarComponent : Component
	{
		enum class FillDirection { LeftToRight, RightToLeft, BottomToTop, TopToBottom };

		float         Value = 1.0f;
		float         MinValue = 0.0f;
		float         MaxValue = 1.0f;

		glm::vec4     ForegroundColor = { 0.20f, 0.80f, 0.20f, 1.0f };
		glm::vec4     BackgroundColor = { 0.15f, 0.15f, 0.15f, 1.0f };
		FillDirection Direction = FillDirection::LeftToRight;
		bool          Visible = true;

		float GetNormalizedValue() const
		{
			if (MaxValue <= MinValue) return 0.0f;
			return glm::clamp((Value - MinValue) / (MaxValue - MinValue), 0.0f, 1.0f);
		}
	};
}