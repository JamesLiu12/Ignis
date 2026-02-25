#include "SceneSerializer.h"
#include "Components.h"
#include "Scene.h"
#include "Ignis/Core/File/File.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

namespace ignis
{
	static ordered_json SerializeVec3(const glm::vec3& vec)
	{
		return ordered_json({ vec.x, vec.y, vec.z });
	}

	static glm::vec3 DeserializeVec3(const ordered_json& data)
	{
		return glm::vec3(data[0], data[1], data[2]);
	}

	static ordered_json SerializeEnvironment(const Environment& env)
	{
		ordered_json env_data;

		if (env.GetSkyboxMap().has_value())
		{
			env_data["SkyboxMap"] = env.GetSkyboxMap().value().ToString();
		}

		if (env.GetIBLMaps().has_value())
		{
			const auto& ibl = env.GetIBLMaps().value();
			ordered_json ibl_data;
			ibl_data["IrradianceMap"] = ibl.IrradianceMap.ToString();
			ibl_data["PrefilteredMap"] = ibl.PrefilteredMap.ToString();
			env_data["IBLMaps"] = ibl_data;
		}

		return env_data;
	}

	static void DeserializeEnvironment(Environment& env, const json& env_data)
	{
		if (env_data.contains("SkyboxMap"))
		{
			env.SetSkyboxMap(UUID(env_data["SkyboxMap"]));
		}

		if (env_data.contains("IBLMaps"))
		{
			const auto& ibl_data = env_data["IBLMaps"];
			IBLMaps ibl;
			ibl.IrradianceMap = UUID(ibl_data["IrradianceMap"]);
			ibl.PrefilteredMap = UUID(ibl_data["PrefilteredMap"]);
			env.SetIBLMaps(ibl);
		}
	}

	static ordered_json SerializeEntity(const Scene& scene, entt::entity entity_handle)
	{
		ordered_json entity_data;
		Entity entity(entity_handle, const_cast<Scene*>(&scene));

		if (entity.HasComponent<IDComponent>())
		{
			const auto& id = entity.GetComponent<IDComponent>();
			entity_data["ID"] = id.ID.ToString();
		}

		if (entity.HasComponent<TagComponent>())
		{
			const auto& tag = entity.GetComponent<TagComponent>();
			entity_data["Tag"] = tag.Tag;
		}

		if (entity.HasComponent<RelationshipComponent>())
		{
			const auto& relationship = entity.GetComponent<RelationshipComponent>();
			ordered_json rel_data;
			rel_data["ParentID"] = relationship.ParentID.ToString();
			rel_data["FirstChildID"] = relationship.FirstChildID.ToString();
			rel_data["LastChildID"] = relationship.LastChildID.ToString();
			rel_data["PrevSiblingID"] = relationship.PrevSiblingID.ToString();
			rel_data["NextSiblingID"] = relationship.NextSiblingID.ToString();
			rel_data["ChildrenCount"] = relationship.ChildrenCount;
			entity_data["Relationship"] = rel_data;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			const auto& transform = entity.GetComponent<TransformComponent>();
			ordered_json transform_data;
			transform_data["Translation"] = SerializeVec3(transform.Translation);
			transform_data["Rotation"] = SerializeVec3(transform.Rotation);
			transform_data["Scale"] = SerializeVec3(transform.Scale);
			entity_data["Transform"] = transform_data;
		}

		if (entity.HasComponent<DirectionalLightComponent>())
		{
			const auto& light = entity.GetComponent<DirectionalLightComponent>();
			ordered_json light_data;
			light_data["Color"] = SerializeVec3(light.Color);
			light_data["Intensity"] = light.Intensity;
			entity_data["DirectionalLight"] = light_data;
		}

		if (entity.HasComponent<PointLightComponent>())
		{
			const auto& light = entity.GetComponent<PointLightComponent>();
			ordered_json light_data;
			light_data["Color"] = SerializeVec3(light.Color);
			light_data["Intensity"] = light.Intensity;
			light_data["Range"] = light.Range;
			entity_data["PointLight"] = light_data;
		}

		if (entity.HasComponent<SpotLightComponent>())
		{
			const auto& light = entity.GetComponent<SpotLightComponent>();
			ordered_json light_data;
			light_data["Color"] = SerializeVec3(light.Color);
			light_data["Intensity"] = light.Intensity;
			light_data["Range"] = light.Range;
			light_data["InnerConeAngle"] = light.InnerConeAngle;
			light_data["OuterConeAngle"] = light.OuterConeAngle;
			entity_data["SpotLight"] = light_data;
		}

		if (entity.HasComponent<SkyLightComponent>())
		{
			const auto& skylight = entity.GetComponent<SkyLightComponent>();
			ordered_json skylight_data;
			skylight_data["Environment"] = SerializeEnvironment(skylight.SceneEnvironment);
			skylight_data["Intensity"] = skylight.Intensity;
			skylight_data["Rotation"] = skylight.Rotation;
			skylight_data["Tint"] = SerializeVec3(skylight.Tint);
			skylight_data["SkyboxLod"] = skylight.SkyboxLod;
			entity_data["SkyLight"] = skylight_data;
		}

		return entity_data;
	}

	bool SceneSerializer::Serialize(const Scene& scene, const std::filesystem::path& filepath)
	{
		File file(filepath);
		auto stream = file.OpenOutputStream();
		if (!stream.is_open())
		{
			return false;
			Log::CoreError("[SceneSerializer::Serialize] Failed to open file for writing");
		}

		ordered_json data;
		data["SceneName"] = scene.GetName();
		data["Entities"] = ordered_json::array();

		std::vector<entt::entity> entities;
		auto view = scene.m_registry.view<IDComponent>();
		entities.reserve(view.size());

		for (const auto& entity : view)
		{
			entities.push_back(entity);
		}

		std::sort(entities.begin(), entities.end(), [&scene](entt::entity lhs, entt::entity rhs) {
			const auto& lhs_id = scene.m_registry.get<IDComponent>(lhs);
			const auto& rhs_id = scene.m_registry.get<IDComponent>(rhs);
			return lhs_id.ID < rhs_id.ID;
		});

		for (const auto& entity : entities)
		{
			data["Entities"].push_back(SerializeEntity(scene, entity));
		}

		try
		{
			stream << data.dump(4);
			stream.close();
			Log::CoreInfo("[SceneSerializer::Serialize] Successfully serialized scene to: {}", file.GetPath().string());
			return true;
		}
		catch (const std::exception& e)
		{
			Log::CoreError("[SceneSerializer::Serialize] Failed to write JSON: {}", e.what());
			return false;
		}
	}

	static Entity DeserializeEntity(Scene& scene, const json& entity_data)
	{
		UUID id = UUID(entity_data.value("ID", ""));
		std::string tag = entity_data.value("Tag", "Entity");

		Entity entity = scene.CreateEntityWithID(id, tag);

		if (entity_data.contains("Relationship"))
		{
			const auto& rel_data = entity_data["Relationship"];
			auto& relationship = entity.GetComponent<RelationshipComponent>();
			relationship.ParentID = UUID(rel_data.value("ParentID", ""));
			relationship.FirstChildID = UUID(rel_data.value("FirstChildID", ""));
			relationship.LastChildID = UUID(rel_data.value("LastChildID", ""));
			relationship.PrevSiblingID = UUID(rel_data.value("PrevSiblingID", ""));
			relationship.NextSiblingID = UUID(rel_data.value("NextSiblingID", ""));
			relationship.ChildrenCount = rel_data.value("ChildrenCount", 0u);
		}

		if (entity_data.contains("Transform"))
		{
			const auto& transform_data = entity_data["Transform"];
			auto& transform = entity.AddComponent<TransformComponent>();
			transform.Translation = DeserializeVec3(transform_data["Translation"]);
			transform.Rotation = DeserializeVec3(transform_data["Rotation"]);
			transform.Scale = DeserializeVec3(transform_data["Scale"]);
		}

		if (entity_data.contains("DirectionalLight"))
		{
			const auto& light_data = entity_data["DirectionalLight"];
			auto& light = entity.AddComponent<DirectionalLightComponent>();
			light.Color = DeserializeVec3(light_data["Color"]);
			light.Intensity = light_data["Intensity"];
		}

		if (entity_data.contains("PointLight"))
		{
			const auto& light_data = entity_data["PointLight"];
			auto& light = entity.AddComponent<PointLightComponent>();
			light.Color = DeserializeVec3(light_data["Color"]);
			light.Intensity = light_data["Intensity"];
			light.Range = light_data["Range"];
		}

		if (entity_data.contains("SpotLight"))
		{
			const auto& light_data = entity_data["SpotLight"];
			auto& light = entity.AddComponent<SpotLightComponent>();
			light.Color = DeserializeVec3(light_data["Color"]);
			light.Intensity = light_data["Intensity"];
			light.Range = light_data["Range"];
			light.InnerConeAngle = light_data["InnerConeAngle"];
			light.OuterConeAngle = light_data["OuterConeAngle"];
		}

		if (entity_data.contains("SkyLight"))
		{
			const auto& skylight_data = entity_data["SkyLight"];
			auto& skylight = entity.AddComponent<SkyLightComponent>();
			if (skylight_data.contains("Environment"))
			{
				DeserializeEnvironment(skylight.SceneEnvironment, skylight_data["Environment"]);
			}
			skylight.Intensity = skylight_data["Intensity"];
			skylight.Rotation = skylight_data["Rotation"];
			skylight.Tint = DeserializeVec3(skylight_data["Tint"]);
			skylight.SkyboxLod = skylight_data["SkyboxLod"];
		}

		return entity;
	}

	std::shared_ptr<Scene> SceneSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		File file(filepath);
		auto stream = file.OpenInputStream();
		if (!stream.is_open())
		{
			Log::CoreError("[SceneSerializer::Deserialize] Failed to open file for reading: {}", filepath.string());
			return nullptr;
		}

		json data;
		try
		{
			stream >> data;
		}
		catch (const std::exception& e)
		{
			Log::CoreError("[SceneSerializer::Deserialize] Failed to parse JSON: {}", e.what());
			return nullptr;
		}

		std::string scene_name = data.value("SceneName", "UntitledScene");
		auto scene = std::make_shared<Scene>(scene_name);

		if (data.contains("Entities") && data["Entities"].is_array())
		{
			for (const auto& entity_data : data["Entities"])
			{
				DeserializeEntity(*scene, entity_data);
			}
		}

		Log::CoreInfo("[SceneSerializer::Deserialize] Successfully deserialized scene from: {}", filepath.string());
		return scene;
	}
}