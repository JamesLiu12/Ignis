#pragma once

#include "Ignis/Core/UUID.h"
#include "Ignis/Renderer/Material.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ignis
{
	struct Component
	{
		virtual ~Component() = 0;
	};

	inline Component::~Component() = default;

	struct IDComponent : Component
	{
		UUID ID = UUID::Invalid;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID id)
			: ID(id) {}
	};

	struct TagComponent : Component
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	struct RelationshipComponent : Component
	{
		UUID ParentID = UUID::Invalid;
		UUID FirstChildID = UUID::Invalid;
		UUID LastChildID = UUID::Invalid;
		UUID PrevSiblingID = UUID::Invalid;
		UUID NextSiblingID = UUID::Invalid;

		uint32_t ChildrenCount = 0;
	};

	struct TransformComponent : Component
	{
		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 Rotation = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation, const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f))
			: Translation(translation), Rotation(rotation), Scale(scale) {}

		glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.0f), Translation)
				* glm::mat4_cast(glm::quat(Rotation))
				* glm::scale(glm::mat4(1.0f), Scale);
		}

		glm::quat GetRotationQuat() const
		{
			return glm::quat(Rotation);
		}
	};

	// Directional Light Component
	struct DirectionalLightComponent : Component
	{
		glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
		float Intensity = 1.0f;
		
		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
	};
	
	// Point Light Component
	struct PointLightComponent : Component
	{
		glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
		float Intensity = 1.0f;
		float Range = 10.0f;
		
		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};
	
	// Spot Light Component
	struct SpotLightComponent : Component
	{
		glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
		float Intensity = 1.0f;
		float Range = 10.0f;
		float InnerConeAngle = 12.5f; //deg
		float OuterConeAngle = 17.5f; //deg
		
		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent&) = default;
	};

	struct SkyLightComponent : Component
	{
		AssetHandle SceneEnvironment;

		float Intensity = 1.0f;
		float Rotation = 0.0f;
		glm::vec3 Tint = { 1.0f, 1.0f, 1.0f };
		float SkyboxLod = 0.0f;
	};

	struct MeshComponent : Component
	{
		AssetHandle Mesh;
		MaterialData MeterialData;
	};
}