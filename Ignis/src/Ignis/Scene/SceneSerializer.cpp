#include "SceneSerializer.h"
#include "Components.h"
#include "Scene.h"
#include "Ignis/Core/File/File.h"
#include "Ignis/Renderer/Material.h"
#include "Ignis/UI/UIComponents.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

namespace ignis
{
	static ordered_json SerializeVec2(const glm::vec2& vec)
	{
		return ordered_json({ vec.x, vec.y });
	}

	static glm::vec2 DeserializeVec2(const ordered_json& data)
	{
		return glm::vec2(data[0], data[1]);
	}

	static ordered_json SerializeVec3(const glm::vec3& vec)
	{
		return ordered_json({ vec.x, vec.y, vec.z });
	}

	static glm::vec3 DeserializeVec3(const ordered_json& data)
	{
		return glm::vec3(data[0], data[1], data[2]);
	}

	static ordered_json SerializeVec4(const glm::vec4& vec)
	{
		return ordered_json({ vec.x, vec.y, vec.z, vec.w });
	}

	static glm::vec4 DeserializeVec4(const ordered_json& data)
	{
		return glm::vec4(data[0], data[1], data[2], data[3]);
	}

	static MaterialData DeserializeMaterialData(const json& data)
	{
		MaterialData material_data{};

		material_data.AlbedoMap = UUID(data.value("AlbedoMap", ""));
		material_data.AlbedoColor = data.contains("AlbedoColor")
			? DeserializeVec4(data["AlbedoColor"])
			: glm::vec4(1.0f);

		material_data.NormalMap = UUID(data.value("NormalMap", ""));

		material_data.MetalnessMap = UUID(data.value("MetalnessMap", ""));
		material_data.MetallicValue = data.value("MetallicValue", 0.0f);

		material_data.RoughnessMap = UUID(data.value("RoughnessMap", ""));
		material_data.RoughnessValue = data.value("RoughnessValue", 0.5f);

		material_data.EmissiveMap = UUID(data.value("EmissiveMap", ""));
		material_data.EmissiveColor = data.contains("EmissiveColor")
			? DeserializeVec3(data["EmissiveColor"])
			: glm::vec3(0.0f);
		material_data.EmissiveIntensity = data.value("EmissiveIntensity", 1.0f);

		material_data.AOMap = UUID(data.value("AOMap", ""));

		material_data.ClearcoatFactor = data.value("ClearcoatFactor", 0.0f);
		material_data.ClearcoatRoughnessFactor = data.value("ClearcoatRoughnessFactor", 0.0f);
		material_data.ClearcoatMap = UUID(data.value("ClearcoatMap", ""));
		material_data.ClearcoatRoughnessMap = UUID(data.value("ClearcoatRoughnessMap", ""));
		material_data.ClearcoatNormalMap = UUID(data.value("ClearcoatNormalMap", ""));

		return material_data;
	}

	static ordered_json SerializeMaterialData(const MaterialData& material_data)
	{
		ordered_json data;

		data["AlbedoMap"] = material_data.AlbedoMap.ToString();
		data["AlbedoColor"] = SerializeVec4(material_data.AlbedoColor);

		data["NormalMap"] = material_data.NormalMap.ToString();

		data["MetalnessMap"] = material_data.MetalnessMap.ToString();
		data["MetallicValue"] = material_data.MetallicValue;

		data["RoughnessMap"] = material_data.RoughnessMap.ToString();
		data["RoughnessValue"] = material_data.RoughnessValue;

		data["EmissiveMap"] = material_data.EmissiveMap.ToString();
		data["EmissiveColor"] = SerializeVec3(material_data.EmissiveColor);
		data["EmissiveIntensity"] = material_data.EmissiveIntensity;

		data["AOMap"] = material_data.AOMap.ToString();

		data["ClearcoatFactor"] = material_data.ClearcoatFactor;
		data["ClearcoatRoughnessFactor"] = material_data.ClearcoatRoughnessFactor;
		data["ClearcoatMap"] = material_data.ClearcoatMap.ToString();
		data["ClearcoatRoughnessMap"] = material_data.ClearcoatRoughnessMap.ToString();
		data["ClearcoatNormalMap"] = material_data.ClearcoatNormalMap.ToString();

		return data;
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

		if (entity.HasComponent<CameraComponent>())
		{
			const auto& cam_comp = entity.GetComponent<CameraComponent>();
			const auto& cam = *cam_comp.Camera;
			ordered_json cam_data;

			cam_data["Primary"] = cam_comp.Primary;
			cam_data["FixedAspectRatio"] = cam_comp.FixedAspectRatio;
			cam_data["ProjectionType"] = static_cast<int>(cam.GetProjectionType());
			cam_data["AspectRatio"] = cam.GetAspectRatio();
			cam_data["PerspectiveFOV"] = cam.GetPerspectiveFOV();
			cam_data["PerspectiveNear"] = cam.GetPerspectiveNearClip();
			cam_data["PerspectiveFar"] = cam.GetPerspectiveFarClip();
			cam_data["OrthographicSize"] = cam.GetOrthographicSize();
			cam_data["OrthographicNear"] = cam.GetOrthographicNearClip();
			cam_data["OrthographicFar"] = cam.GetOrthographicFarClip();

			entity_data["Camera"] = cam_data;
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
			skylight_data["Environment"] = skylight.SceneEnvironment.ToString();
			skylight_data["Intensity"] = skylight.Intensity;
			skylight_data["Rotation"] = skylight.Rotation;
			skylight_data["Tint"] = SerializeVec3(skylight.Tint);
			skylight_data["SkyboxLod"] = skylight.SkyboxLod;
			entity_data["SkyLight"] = skylight_data;
		}

		if (entity.HasComponent<MeshComponent>())
		{
			const auto& mesh = entity.GetComponent<MeshComponent>();
			ordered_json mesh_data;
			mesh_data["Mesh"] = mesh.Mesh.ToString();

			ordered_json slots = ordered_json::array();
			for (const auto& slot : mesh.MaterialSlots)
			{
				slots.push_back(SerializeMaterialData(slot));
			}
			mesh_data["MaterialSlots"] = slots;

			entity_data["Mesh"] = mesh_data;
		}

		if (entity.HasComponent<ScriptComponent>())
		{
			const auto& script = entity.GetComponent<ScriptComponent>();
			ordered_json script_data;
			script_data["ClassName"] = script.ClassName;
			script_data["Enabled"] = script.Enabled;
			entity_data["Script"] = script_data;
		}

		if (entity.HasComponent<TextComponent>())
		{
			const auto& text_com = entity.GetComponent<TextComponent>();
			ordered_json text_data;
			text_data["Text"] = text_com.Text;
			text_data["Font"] = text_com.Font.ToString();
			text_data["Color"] = SerializeVec3(text_com.Color);
			text_data["Alpha"] = text_com.Alpha;
			text_data["Scale"] = text_com.Scale;
			entity_data["Text"] = text_data;
		}

		if (entity.HasComponent<AudioSourceComponent>())
		{
			const auto& audio = entity.GetComponent<AudioSourceComponent>();
			ordered_json audio_data;
			audio_data["Clip"] = audio.Clip.ToString();
			audio_data["Volume"] = audio.Volume;
			audio_data["Pitch"] = audio.Pitch;
			audio_data["Loop"] = audio.Loop;
			audio_data["PlayOnStart"] = audio.PlayOnStart;
			audio_data["Spatial"] = audio.Spatial;
			audio_data["MinDistance"] = audio.MinDistance;
			audio_data["MaxDistance"] = audio.MaxDistance;
			entity_data["AudioSource"] = audio_data;
		}

		if (entity.HasComponent<AudioListenerComponent>())
		{
			const auto& listener = entity.GetComponent<AudioListenerComponent>();
			ordered_json listener_data;
			listener_data["Primary"] = listener.Primary;
			entity_data["AudioListener"] = listener_data;
		}

		if (entity.HasComponent<RigidBodyComponent>())
		{
			const auto& rb = entity.GetComponent<RigidBodyComponent>();
			ordered_json rb_data;
			rb_data["BodyType"] = static_cast<int>(rb.BodyType);
			rb_data["Mass"] = rb.Mass;
			rb_data["LinearDrag"] = rb.LinearDrag;
			rb_data["AngularDrag"] = rb.AngularDrag;
			rb_data["UseGravity"] = rb.UseGravity;
			rb_data["IsKinematic"] = rb.IsKinematic;
			rb_data["LockPositionX"] = rb.LockPositionX;
			rb_data["LockPositionY"] = rb.LockPositionY;
			rb_data["LockPositionZ"] = rb.LockPositionZ;
			rb_data["LockRotationX"] = rb.LockRotationX;
			rb_data["LockRotationY"] = rb.LockRotationY;
			rb_data["LockRotationZ"] = rb.LockRotationZ;
			entity_data["RigidBody"] = rb_data;
		}

		if (entity.HasComponent<BoxColliderComponent>())
		{
			const auto& box = entity.GetComponent<BoxColliderComponent>();
			ordered_json box_data;
			box_data["HalfSize"] = SerializeVec3(box.HalfSize);
			box_data["Offset"] = SerializeVec3(box.Offset);
			box_data["Friction"] = box.Material.Friction;
			box_data["Restitution"] = box.Material.Restitution;
			box_data["IsTrigger"] = box.IsTrigger;
			entity_data["BoxCollider"] = box_data;
		}

		if (entity.HasComponent<SphereColliderComponent>())
		{
			const auto& sphere = entity.GetComponent<SphereColliderComponent>();
			ordered_json sphere_data;
			sphere_data["Radius"] = sphere.Radius;
			sphere_data["Offset"] = SerializeVec3(sphere.Offset);
			sphere_data["Friction"] = sphere.Material.Friction;
			sphere_data["Restitution"] = sphere.Material.Restitution;
			sphere_data["IsTrigger"] = sphere.IsTrigger;
			entity_data["SphereCollider"] = sphere_data;
		}

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			const auto& capsule = entity.GetComponent<CapsuleColliderComponent>();
			ordered_json capsule_data;
			capsule_data["Radius"] = capsule.Radius;
			capsule_data["HalfHeight"] = capsule.HalfHeight;
			capsule_data["Offset"] = SerializeVec3(capsule.Offset);
			capsule_data["Friction"] = capsule.Material.Friction;
			capsule_data["Restitution"] = capsule.Material.Restitution;
			capsule_data["IsTrigger"] = capsule.IsTrigger;
			entity_data["CapsuleCollider"] = capsule_data;
		}

		if (entity.HasComponent<RectTransformComponent>())
		{
			const auto& rect = entity.GetComponent<RectTransformComponent>();
			ordered_json rect_data;
			// ResolvedMin / ResolvedMax are runtime-only, skip them
			rect_data["AnchorMin"] = SerializeVec2(rect.AnchorMin);
			rect_data["AnchorMax"] = SerializeVec2(rect.AnchorMax);
			rect_data["OffsetMin"] = SerializeVec2(rect.OffsetMin);
			rect_data["OffsetMax"] = SerializeVec2(rect.OffsetMax);
			entity_data["RectTransform"] = rect_data;
		}

		if (entity.HasComponent<CanvasComponent>())
		{
			const auto& canvas = entity.GetComponent<CanvasComponent>();
			ordered_json canvas_data;
			canvas_data["RenderMode"] = static_cast<int>(canvas.Mode);
			canvas_data["SortOrder"] = canvas.SortOrder;
			canvas_data["Visible"] = canvas.Visible;
			entity_data["Canvas"] = canvas_data;
		}

		if (entity.HasComponent<ImageComponent>())
		{
			const auto& image = entity.GetComponent<ImageComponent>();
			ordered_json image_data;
			image_data["Texture"] = image.Texture.ToString();
			image_data["Color"] = SerializeVec4(image.Color);
			image_data["Visible"] = image.Visible;
			image_data["RaycastTarget"] = image.RaycastTarget;
			image_data["ScaleMode"] = static_cast<int>(image.Scale);
			entity_data["Image"] = image_data;
		}

		if (entity.HasComponent<UITextComponent>())
		{
			const auto& ui_text = entity.GetComponent<UITextComponent>();
			ordered_json ui_text_data;
			ui_text_data["Text"] = ui_text.Text;
			ui_text_data["Font"] = ui_text.Font.ToString();
			ui_text_data["Color"] = SerializeVec4(ui_text.Color);
			ui_text_data["FontSize"] = ui_text.FontSize;
			ui_text_data["HAlign"] = static_cast<int>(ui_text.HAlign);
			ui_text_data["VAlign"] = static_cast<int>(ui_text.VAlign);
			ui_text_data["Visible"] = ui_text.Visible;
			entity_data["UIText"] = ui_text_data;
		}

		if (entity.HasComponent<ButtonComponent>())
		{
			const auto& button = entity.GetComponent<ButtonComponent>();
			ordered_json button_data;
			// IsHovered / IsPressed / CurrentColor are runtime-only, skip them
			button_data["NormalColor"] = SerializeVec4(button.NormalColor);
			button_data["HoverColor"] = SerializeVec4(button.HoverColor);
			button_data["PressedColor"] = SerializeVec4(button.PressedColor);
			button_data["DisabledColor"] = SerializeVec4(button.DisabledColor);
			button_data["Interactable"] = button.Interactable;
			entity_data["Button"] = button_data;
		}

		if (entity.HasComponent<ProgressBarComponent>())
		{
			const auto& bar = entity.GetComponent<ProgressBarComponent>();
			ordered_json bar_data;
			bar_data["Value"] = bar.Value;
			bar_data["MinValue"] = bar.MinValue;
			bar_data["MaxValue"] = bar.MaxValue;
			bar_data["ForegroundColor"] = SerializeVec4(bar.ForegroundColor);
			bar_data["BackgroundColor"] = SerializeVec4(bar.BackgroundColor);
			bar_data["FillDirection"] = static_cast<int>(bar.Direction);
			bar_data["Visible"] = bar.Visible;
			entity_data["ProgressBar"] = bar_data;
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
			auto& transform = entity.GetComponent<TransformComponent>();
			transform.Translation = DeserializeVec3(transform_data["Translation"]);
			transform.Rotation = DeserializeVec3(transform_data["Rotation"]);
			transform.Scale = DeserializeVec3(transform_data["Scale"]);
		}

		if (entity_data.contains("Camera"))
		{
			const auto& cam_data = entity_data["Camera"];
			auto& cam_comp = entity.AddComponent<CameraComponent>();

			cam_comp.Primary = cam_data.value("Primary", true);
			cam_comp.FixedAspectRatio = cam_data.value("FixedAspectRatio", false);

			cam_comp.Camera->SetAspectRatio(cam_data.value("AspectRatio", 16.0f / 9.0f));

			auto type = static_cast<SceneCamera::ProjectionType>(
				cam_data.value("ProjectionType", 0)
				);

			if (type == SceneCamera::ProjectionType::Perspective)
			{
				cam_comp.Camera->SetPerspective(
					cam_data.value("PerspectiveFOV", 45.0f),
					cam_data.value("PerspectiveNear", 0.01f),
					cam_data.value("PerspectiveFar", 1000.0f)
				);
			}
			else
			{
				cam_comp.Camera->SetOrthographic(
					cam_data.value("OrthographicSize", 10.0f),
					cam_data.value("OrthographicNear", -1.0f),
					cam_data.value("OrthographicFar", 1.0f)
				);
			}
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
			skylight.SceneEnvironment = UUID(skylight_data.value("Environment", ""));
			skylight.Intensity = skylight_data["Intensity"];
			skylight.Rotation = skylight_data["Rotation"];
			skylight.Tint = DeserializeVec3(skylight_data["Tint"]);
			skylight.SkyboxLod = skylight_data["SkyboxLod"];
		}

		if (entity_data.contains("Mesh"))
		{
			const auto& mesh_data = entity_data["Mesh"];
			auto& mesh = entity.AddComponent<MeshComponent>();

			mesh.Mesh = mesh_data.contains("Mesh")
				? AssetHandle(mesh_data["Mesh"].get<std::string>())
				: AssetHandle::Invalid;

			// New Format: MaterialSlots Array
			if (mesh_data.contains("MaterialSlots") && mesh_data["MaterialSlots"].is_array())
			{
				for (const auto& slot_data : mesh_data["MaterialSlots"])
				{
					mesh.MaterialSlots.push_back(DeserializeMaterialData(slot_data));
				}
			}
		}

		if (entity_data.contains("Script"))
		{
			const auto& script_data = entity_data["Script"];
			auto& script = entity.AddComponent<ScriptComponent>();
			script.ClassName = script_data.value("ClassName", "");
			script.Enabled = script_data.value("Enabled", false);
		}

		if (entity_data.contains("Text"))
		{
			const auto& text_data = entity_data["Text"];
			auto& text = entity.AddComponent<TextComponent>();
			text.Text = text_data.value("Text", "");
			text.Font = UUID(text_data.value("Font", ""));
			text.Color = DeserializeVec3(text_data["Color"]);
			text.Alpha = text_data.value("Alpha", 1.0f);
			text.Scale = text_data.value("Scale", 1.0f);
		}

		if (entity_data.contains("AudioSource"))
		{
			const auto& audio_data = entity_data["AudioSource"];
			auto& audio = entity.AddComponent<AudioSourceComponent>();
			audio.Clip = AssetHandle(audio_data.value("Clip", ""));
			audio.Volume = audio_data.value("Volume", 1.0f);
			audio.Pitch = audio_data.value("Pitch", 1.0f);
			audio.Loop = audio_data.value("Loop", false);
			audio.PlayOnStart = audio_data.value("PlayOnStart", true);
			audio.Spatial = audio_data.value("Spatial", true);
			audio.MinDistance = audio_data.value("MinDistance", 1.0f);
			audio.MaxDistance = audio_data.value("MaxDistance", 50.0f);
		}

		if (entity_data.contains("AudioListener"))
		{
			const auto& listener_data = entity_data["AudioListener"];
			auto& listener = entity.AddComponent<AudioListenerComponent>();
			listener.Primary = listener_data.value("Primary", true);
		}

		if (entity_data.contains("RigidBody"))
		{
			const auto& rb_data = entity_data["RigidBody"];
			auto& rb = entity.AddComponent<RigidBodyComponent>();
			rb.BodyType = static_cast<BodyType>(rb_data.value("BodyType", 1));
			rb.Mass = rb_data.value("Mass", 1.0f);
			rb.LinearDrag = rb_data.value("LinearDrag", 0.0f);
			rb.AngularDrag = rb_data.value("AngularDrag", 0.05f);
			rb.UseGravity = rb_data.value("UseGravity", true);
			rb.IsKinematic = rb_data.value("IsKinematic", false);
			rb.LockPositionX = rb_data.value("LockPositionX", false);
			rb.LockPositionY = rb_data.value("LockPositionY", false);
			rb.LockPositionZ = rb_data.value("LockPositionZ", false);
			rb.LockRotationX = rb_data.value("LockRotationX", false);
			rb.LockRotationY = rb_data.value("LockRotationY", false);
			rb.LockRotationZ = rb_data.value("LockRotationZ", false);
		}

		if (entity_data.contains("BoxCollider"))
		{
			const auto& box_data = entity_data["BoxCollider"];
			auto& box = entity.AddComponent<BoxColliderComponent>();
			box.HalfSize = DeserializeVec3(box_data["HalfSize"]);
			box.Offset = DeserializeVec3(box_data["Offset"]);
			box.Material.Friction = box_data.value("Friction", 0.5f);
			box.Material.Restitution = box_data.value("Restitution", 0.3f);
			box.IsTrigger = box_data.value("IsTrigger", false);
		}

		if (entity_data.contains("SphereCollider"))
		{
			const auto& sphere_data = entity_data["SphereCollider"];
			auto& sphere = entity.AddComponent<SphereColliderComponent>();
			sphere.Radius = sphere_data.value("Radius", 0.5f);
			sphere.Offset = DeserializeVec3(sphere_data["Offset"]);
			sphere.Material.Friction = sphere_data.value("Friction", 0.5f);
			sphere.Material.Restitution = sphere_data.value("Restitution", 0.3f);
			sphere.IsTrigger = sphere_data.value("IsTrigger", false);
		}

		if (entity_data.contains("CapsuleCollider"))
		{
			const auto& capsule_data = entity_data["CapsuleCollider"];
			auto& capsule = entity.AddComponent<CapsuleColliderComponent>();
			capsule.Radius = capsule_data.value("Radius", 0.5f);
			capsule.HalfHeight = capsule_data.value("HalfHeight", 0.5f);
			capsule.Offset = DeserializeVec3(capsule_data["Offset"]);
			capsule.Material.Friction = capsule_data.value("Friction", 0.5f);
			capsule.Material.Restitution = capsule_data.value("Restitution", 0.3f);
			capsule.IsTrigger = capsule_data.value("IsTrigger", false);
		}

		if (entity_data.contains("RectTransform"))
		{
			const auto& rect_data = entity_data["RectTransform"];
			auto& rect = entity.AddComponent<RectTransformComponent>();
			rect.AnchorMin = DeserializeVec2(rect_data["AnchorMin"]);
			rect.AnchorMax = DeserializeVec2(rect_data["AnchorMax"]);
			rect.OffsetMin = DeserializeVec2(rect_data["OffsetMin"]);
			rect.OffsetMax = DeserializeVec2(rect_data["OffsetMax"]);
		}

		if (entity_data.contains("Canvas"))
		{
			const auto& canvas_data = entity_data["Canvas"];
			auto& canvas = entity.AddComponent<CanvasComponent>();
			canvas.Mode = static_cast<CanvasComponent::RenderMode>(canvas_data.value("RenderMode", 0));
			canvas.SortOrder = canvas_data.value("SortOrder", 0);
			canvas.Visible = canvas_data.value("Visible", true);
		}

		if (entity_data.contains("Image"))
		{
			const auto& image_data = entity_data["Image"];
			auto& image = entity.AddComponent<ImageComponent>();
			image.Texture = AssetHandle(image_data.value("Texture", ""));
			image.Color = DeserializeVec4(image_data["Color"]);
			image.Visible = image_data.value("Visible", true);
			image.RaycastTarget = image_data.value("RaycastTarget", true);
			image.Scale = static_cast<ImageComponent::ScaleMode>(image_data.value("ScaleMode", 0));
		}

		if (entity_data.contains("UIText"))
		{
			const auto& ui_text_data = entity_data["UIText"];
			auto& ui_text = entity.AddComponent<UITextComponent>();
			ui_text.Text = ui_text_data.value("Text", "");
			ui_text.Font = AssetHandle(ui_text_data.value("Font", ""));
			ui_text.Color = DeserializeVec4(ui_text_data["Color"]);
			ui_text.FontSize = ui_text_data.value("FontSize", 16.0f);
			ui_text.HAlign = static_cast<UITextComponent::HorizontalAlignment>(ui_text_data.value("HAlign", 0));
			ui_text.VAlign = static_cast<UITextComponent::VerticalAlignment>(ui_text_data.value("VAlign", 0));
			ui_text.Visible = ui_text_data.value("Visible", true);
		}

		if (entity_data.contains("Button"))
		{
			const auto& button_data = entity_data["Button"];
			auto& button = entity.AddComponent<ButtonComponent>();
			button.NormalColor = DeserializeVec4(button_data["NormalColor"]);
			button.HoverColor = DeserializeVec4(button_data["HoverColor"]);
			button.PressedColor = DeserializeVec4(button_data["PressedColor"]);
			button.DisabledColor = DeserializeVec4(button_data["DisabledColor"]);
			button.Interactable = button_data.value("Interactable", true);
		}

		if (entity_data.contains("ProgressBar"))
		{
			const auto& bar_data = entity_data["ProgressBar"];
			auto& bar = entity.AddComponent<ProgressBarComponent>();
			bar.Value = bar_data.value("Value", 1.0f);
			bar.MinValue = bar_data.value("MinValue", 0.0f);
			bar.MaxValue = bar_data.value("MaxValue", 1.0f);
			bar.ForegroundColor = DeserializeVec4(bar_data["ForegroundColor"]);
			bar.BackgroundColor = DeserializeVec4(bar_data["BackgroundColor"]);
			bar.Direction = static_cast<ProgressBarComponent::FillDirection>(bar_data.value("FillDirection", 0));
			bar.Visible = bar_data.value("Visible", true);
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