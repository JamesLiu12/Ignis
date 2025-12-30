#pragma once

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

	struct TagComponent : Component
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
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
	};

	struct LightComponent : Component
	{
		enum class Type { Directional, Point, Spot };
		
		Type LightType = Type::Directional;
		glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
		float Intensity = 1.0f;
		
		// For directional lights
		glm::vec3 Direction = glm::vec3(-0.2f, -1.0f, -0.3f);
		
		// For point and spot lights
		float Range = 10.0f;
		float Attenuation = 1.0f;
		
		// For spot lights
		float InnerConeAngle = 12.5f; // degrees
		float OuterConeAngle = 17.5f; // degrees
		
		LightComponent() = default;
		LightComponent(const LightComponent&) = default;
		LightComponent(Type type) : LightType(type) {}
	};
}