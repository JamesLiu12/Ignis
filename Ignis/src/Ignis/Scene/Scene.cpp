#include "Scene.h"
#include "Entity.h"
#include "Ignis/Asset/AssetManager.h"
#include "Ignis/Renderer/SceneRenderer.h"
#include "Ignis/Script/ScriptBehaviour.h"
#include "Ignis/Script/ScriptRegistry.h"
#include "Ignis/UI/UIComponents.h"

namespace ignis
{
	// Helper template for copying components from source to destination registry
	template<typename T>
	static void CopyComponent(entt::registry& dst_registry, entt::registry& src_registry, 
	                          const std::unordered_map<UUID, entt::entity>& entt_map)
	{
		auto src_entities = src_registry.view<T>();
		for (auto src_entity : src_entities)
		{
			entt::entity dest_entity = entt_map.at(src_registry.get<IDComponent>(src_entity).ID);
			auto& src_component = src_registry.get<T>(src_entity);
			dst_registry.emplace_or_replace<T>(dest_entity, src_component);
		}
	}
	Entity Scene::CreateEntity(const std::string name)
	{
		return CreateEntity({}, name);
	}

	Entity Scene::CreateEntity(Entity parent, const std::string name)
	{
		Entity entity = Entity(m_registry.create(), this);

		auto& id_component = entity.AddComponent<IDComponent>(UUID());
		m_id_entity_map[id_component.ID] = entity;

		entity.AddComponent<RelationshipComponent>();

		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);

		if (parent.IsValid())
		{
			entity.SetParent(parent);
		}

		return entity;
	}

	Entity Scene::CreateEntityWithID(UUID uuid, const std::string& name)
	{
		return CreateEntityWithID(uuid, {}, name);
	}

	Entity Scene::CreateEntityWithID(UUID uuid, Entity parent, const std::string& name)
	{
		Entity entity = Entity(m_registry.create(), this);

		auto& id_component = entity.AddComponent<IDComponent>(uuid);
		m_id_entity_map[id_component.ID] = entity;

		entity.AddComponent<RelationshipComponent>();

		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);

		if (parent.IsValid())
		{
			entity.SetParent(parent);
		}

		return entity;
	}

	Entity Scene::GetEntityByHandle(entt::entity handle)
	{
		return Entity(handle, this);
	}
	
	Entity Scene::GetEntityByID(UUID id) const
	{
		const auto it = m_id_entity_map.find(id);
		if (it != m_id_entity_map.end()) return it->second;
		return {};
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if (!entity.IsValid())
			return;
		
		UUID entity_id = entity.GetID();
		
		// Recursively delete all children first
		std::vector<Entity> children = entity.GetChildren();
		for (Entity child : children)
		{
			DestroyEntity(child);
		}
		
		// Unparent from parent (updates parent's child list)
		if (entity.GetParent())
		{
			entity.Unparent();
		}
		
		// Remove from ID-entity map
		m_id_entity_map.erase(entity_id);
		
		// Destroy the entity in the registry
		m_registry.destroy(entity.m_handle);
		
		Log::CoreInfo("Scene: Destroyed entity {}", entity_id.ToString());
	}

	std::shared_ptr<Camera> Scene::GetPrimaryCamera()
	{
		std::shared_ptr<Camera> result;

		auto view = m_registry.view<CameraComponent, TransformComponent>();
		view.each([&](auto entity_handle, CameraComponent& camera_component, TransformComponent&)
			{
				if (result) return;

				if (!camera_component.Primary) return;

				Entity entity(entity_handle, this);
				camera_component.Camera->SetViewFromWorldTransform(entity.GetWorldTransform());
				result = camera_component.Camera;
			});

		return result;
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0) return;

		float aspect = static_cast<float>(width) / static_cast<float>(height);
		auto view = m_registry.view<CameraComponent>();
		view.each([aspect](CameraComponent& camera_component)
			{
				if (!camera_component.FixedAspectRatio)
					camera_component.Camera->SetAspectRatio(aspect);
			});
	}

	static inline void ComputeAttenuationFromRange(float range, float& out_linear, float& out_quadratic, float edge = 0.01f)
	{
		range = std::max(range, 1e-4f);
		edge = std::clamp(edge, 1e-6f, 1.0f);

		out_linear = 0.0f;
		out_quadratic = (1.0f / edge - 1.0f) / (range * range);
	}

	void Scene::OnRender(const SceneRenderer& scene_renderer)
	{
		m_light_environment = LightEnvironment();

		// -------------------------
		// Directional
		// -------------------------
		{
			uint32_t light_index = 0;
			auto lights = m_registry.group<DirectionalLightComponent>(entt::get<TransformComponent>);

			lights.each([&](auto entity, DirectionalLightComponent& light, TransformComponent& transform)
				{
					if (light_index >= LightEnvironment::MaxDirectionalLights) return;
					
					glm::vec3 direction = glm::normalize(transform.GetRotationQuat() * glm::vec3(0.0f, 0.0f, -1.0f));

					m_light_environment.DirectionalLights.emplace_back
					(
						direction,
						light.Color * light.Intensity
					);
					light_index++;
				});
		}

		// -------------------------
		// Point
		// -------------------------
		{
			uint32_t light_index = 0;
			auto lights = m_registry.group<PointLightComponent>(entt::get<TransformComponent>);

			lights.each([&](auto /*entity*/, PointLightComponent& light, TransformComponent& transform)
				{
					if (light_index >= LightEnvironment::MaxPointLights) return;

					float linear = 0.0f;
					float quadratic = 0.0f;
					ComputeAttenuationFromRange(light.Range, linear, quadratic);

					m_light_environment.PointLights.emplace_back
					(
						transform.Translation,
						light.Color * light.Intensity,
						1.0f,
						linear,
						quadratic
					);
					light_index++;
				});
		}

		// -------------------------
		// Spot
		// -------------------------
		{
			uint32_t light_index = 0;
			auto lights = m_registry.group<SpotLightComponent>(entt::get<TransformComponent>);

			lights.each([&](auto entity, SpotLightComponent& light, TransformComponent& transform)
				{
					if (light_index >= LightEnvironment::MaxSpotLights) return;

					float linear = 0.0f;
					float quadratic = 0.0f;
					ComputeAttenuationFromRange(light.Range, linear, quadratic);

					float inner = light.InnerConeAngle;
					float outer = light.OuterConeAngle;
					if (inner > outer) std::swap(inner, outer);

					glm::vec3 direction = glm::normalize(transform.GetRotationQuat() * glm::vec3(0.0f, 0.0f, -1.0f));

					m_light_environment.SpotLights.emplace_back
					(
						transform.Translation,
						light.Color * light.Intensity,
						direction,
						1.0f,
						linear,
						quadratic,
						glm::cos(glm::radians(inner)),
						glm::cos(glm::radians(outer))
					);
					light_index++;
				});
		}

		// -------------------------
		// SkyLight
		// -------------------------
		{
			auto sky_lights = m_registry.group<SkyLightComponent>();

			// Clear environment if no SkyLight exists
			if (sky_lights.empty())
			{
				m_scene_environment = nullptr;
			}
			else
			{
				sky_lights.each([&](auto entity, SkyLightComponent& sky_light)
					{
						m_scene_environment = AssetManager::GetAsset<Environment>(sky_light.SceneEnvironment);
						
						// Warn if environment asset is missing
						if (!m_scene_environment)
						{
							Log::CoreWarn("SkyLight references missing Environment asset: {}", 
							              sky_light.SceneEnvironment.ToString());
						}
						
						m_environment_settings.Intensity = sky_light.Intensity;
						m_environment_settings.Rotation = sky_light.Rotation;
						m_environment_settings.Tint = sky_light.Tint;
						m_environment_settings.SkyboxLod = sky_light.SkyboxLod;
					});
			}

			scene_renderer.SubmitSkybox();
		}

		// -------------------------
		// Mesh
		// -------------------------
		{
			auto meshes = m_registry.group<MeshComponent>(entt::get<TransformComponent>);

			meshes.each([&](auto entity_handle, MeshComponent& mesh_component, TransformComponent& transform)
				{
					if (auto mesh = AssetManager::GetAsset<Mesh>(mesh_component.Mesh))
					{
						const uint32_t slot_count = static_cast<uint32_t>(mesh_component.MaterialSlots.size());
						for (uint32_t i = 0; i < slot_count; i++)
						{
							mesh->SetMaterialData(i, mesh_component.MaterialSlots[i]);
						}

						Entity entity(entity_handle, this);
						scene_renderer.SubmitMesh(*mesh, entity.GetWorldTransform());
					}
				});
		}

		// -------------------------
		// Text
		// -------------------------
		{
			auto texts = m_registry.group<TextComponent>(entt::get<TransformComponent>);

			texts.each([&](auto entity_handle, TextComponent& text_component, TransformComponent& transform)
				{
					if (auto font = AssetManager::GetAsset<Font>(text_component.Font))
					{
						Entity entity(entity_handle, this);
						scene_renderer.SubmitText(
							*font,
							text_component.Text,
							entity.GetWorldTransform(),
							glm::vec4(text_component.Color, text_component.Alpha),
							text_component.Scale
						);
					}
				});
		}
	}

	void Scene::OnRuntimeStart()
	{
		OnRuntimeStop();

		m_audio_system = std::make_unique<AudioSystem>(this);
		m_audio_system->OnStart();

		auto scripts = m_registry.group<ScriptComponent>(entt::get<IDComponent>);

		scripts.each([&](entt::entity entity_handle, ScriptComponent& script_component, IDComponent& id_component)
			{
				Entity entity(entity_handle, this);

				if (!script_component.Enabled)
					return;

				auto script_behaviour = ScriptRegistry::Get().Create(script_component.ClassName);

				if (!script_behaviour)
				{
					Log::CoreWarn("[Scene::OnRuntimeStart] Invalid ScriptAsset on entity {}", entity.GetID().ToString());
					return;
				}

				auto [it, inserted] = m_runtime_scripts.try_emplace(id_component.ID, std::move(script_behaviour));
				auto& script = it->second;

				script.GetBehaviour().SetEntity(entity);
				script.GetBehaviour().OnCreate();
			});

		// Initialize physics world
		m_physics_world = std::make_unique<PhysicsWorld>();
		m_physics_world->Init();
		
		// Create physics bodies for all entities with RigidBodyComponent
		CreatePhysicsBodies();
	}

	void Scene::OnRuntimeUpdate(float dt)
	{
		// Sync entity transforms to physics (for kinematic bodies)
		SyncTransformsToPhysics();
		
		// Step physics simulation
		if (m_physics_world)
			m_physics_world->Step(dt);
		
		// Sync physics transforms back to entities (for dynamic bodies)
		SyncTransformsFromPhysics();

		auto scripts = m_registry.group<ScriptComponent>(entt::get<IDComponent>);

		scripts.each([&](entt::entity entity_handle, ScriptComponent& script_component, IDComponent& id_component)
			{
				if (!script_component.Enabled)
					return;

				auto it = m_runtime_scripts.find(id_component.ID);
				if (it == m_runtime_scripts.end())
					return;

				it->second.GetBehaviour().OnUpdate(dt);
			});

		auto cameras = m_registry.view<CameraComponent, TransformComponent>();
		cameras.each([&](entt::entity entity_handle, CameraComponent& camera_component, TransformComponent&)
			{
				Entity entity(entity_handle, this);
				camera_component.Camera->SetViewFromWorldTransform(entity.GetWorldTransform());
			});

		if (m_audio_system)
			m_audio_system->OnUpdate(dt);
	}

	void Scene::OnRuntimeStop()
	{
		// Clear runtime_body pointers from components BEFORE shutting down physics
		auto rb_view = m_registry.view<RigidBodyComponent>();
		for (auto entity : rb_view)
		{
			auto& rb = rb_view.get<RigidBodyComponent>(entity);
			rb.runtime_body.reset();
		}
		
		// Cleanup physics
		if (m_physics_world)
		{
			m_physics_world->Shutdown();
			m_physics_world.reset();
		}

		if (m_audio_system)
		{
			m_audio_system->OnStop();
			m_audio_system.reset();
		}

		auto scripts = m_registry.group<ScriptComponent>(entt::get<IDComponent>);

		scripts.each([&](entt::entity entity_handle, ScriptComponent& script_component, IDComponent& id_component)
			{
				if (!script_component.Enabled)
					return;

				auto it = m_runtime_scripts.find(id_component.ID);
				if (it == m_runtime_scripts.end())
					return;

				it->second.GetBehaviour().OnDestroy();
			});

		m_runtime_scripts.clear();
	}

	void Scene::CopyTo(std::shared_ptr<Scene>& target)
	{
		// Copy scene name
		target->m_name = m_name;
		
		// Copy environment settings
		target->m_scene_environment = m_scene_environment;
		target->m_environment_settings = m_environment_settings;
		
		// Step 1: Create all entities with preserved UUIDs in consistent order
		std::unordered_map<UUID, entt::entity> entt_map;
		auto id_components = m_registry.view<IDComponent>();
		
		// Sort entities by their entt::entity handle to ensure consistent creation order
		std::vector<entt::entity> sorted_entities;
		sorted_entities.reserve(id_components.size());
		for (auto entity : id_components)
		{
			sorted_entities.push_back(entity);
		}
		std::sort(sorted_entities.begin(), sorted_entities.end());
		
		// Create entities in sorted order
		for (auto entity : sorted_entities)
		{
			auto uuid = m_registry.get<IDComponent>(entity).ID;
			auto name = m_registry.get<TagComponent>(entity).Tag;
			Entity e = target->CreateEntityWithID(uuid, name);
			entt_map[uuid] = e.m_handle;
		}
		
		// Step 2: Copy all component types
		CopyComponent<IDComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<TagComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<TransformComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<RelationshipComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<CameraComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<DirectionalLightComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<PointLightComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<SpotLightComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<SkyLightComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<MeshComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<ScriptComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<TextComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<RectTransformComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<CanvasComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<ImageComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<UITextComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<ButtonComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<ProgressBarComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<AudioSourceComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<AudioListenerComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<RigidBodyComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<BoxColliderComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<SphereColliderComponent>(target->m_registry, m_registry, entt_map);
		CopyComponent<CapsuleColliderComponent>(target->m_registry, m_registry, entt_map);
		
		// Step 3: Deep copy camera instances (avoid shared camera between scenes)
		auto cameras = m_registry.view<CameraComponent>();
		for (auto src_entity : cameras)
		{
			entt::entity dest_entity = entt_map.at(m_registry.get<IDComponent>(src_entity).ID);
			auto& src_camera = m_registry.get<CameraComponent>(src_entity);
			auto& dest_camera = target->m_registry.get<CameraComponent>(dest_entity);
			
			// Create new camera instance with same settings
			dest_camera.Camera = std::make_shared<SceneCamera>(*src_camera.Camera);
		}
		
		Log::CoreInfo("Scene::CopyTo() - Copied scene '{}' with {} entities", 
		              m_name, entt_map.size());
	}

	ScriptBehaviour* Scene::GetRuntimeScript(UUID entity_id)
	{
		auto it = m_runtime_scripts.find(entity_id);
		if (it == m_runtime_scripts.end()) return nullptr;
		return &it->second.GetBehaviour();
	}

	void Scene::CreatePhysicsBodies()
	{
		auto view = m_registry.view<RigidBodyComponent, TransformComponent>();
		
		for (auto entity_handle : view)
		{
			Entity entity(entity_handle, this);
			auto& rb = view.get<RigidBodyComponent>(entity_handle);
			auto& transform = view.get<TransformComponent>(entity_handle);
			
			// Build RigidBodyDesc from component data
			RigidBodyDesc desc;
			desc.type = rb.body_type;
			desc.position = transform.Translation;
			desc.rotation = glm::quat(glm::radians(transform.Rotation));
			desc.mass = rb.mass;
			
			// Determine shape and size from collider components
			bool has_collider = false;
			
			if (entity.HasComponent<BoxColliderComponent>())
			{
				auto& box = entity.GetComponent<BoxColliderComponent>();
				desc.shape = ShapeType::Box;
				desc.size = box.half_size * 2.0f;
				desc.friction = box.material.friction;
				desc.restitution = box.material.restitution;
				has_collider = true;
			}
			else if (entity.HasComponent<SphereColliderComponent>())
			{
				auto& sphere = entity.GetComponent<SphereColliderComponent>();
				desc.shape = ShapeType::Sphere;
				desc.size = glm::vec3(sphere.radius * 2.0f);
				desc.friction = sphere.material.friction;
				desc.restitution = sphere.material.restitution;
				has_collider = true;
			}
			else if (entity.HasComponent<CapsuleColliderComponent>())
			{
				auto& capsule = entity.GetComponent<CapsuleColliderComponent>();
				desc.shape = ShapeType::Capsule;
				desc.size = glm::vec3(capsule.radius * 2.0f, capsule.half_height * 2.0f, 0.0f);
				desc.friction = capsule.material.friction;
				desc.restitution = capsule.material.restitution;
				has_collider = true;
			}
			
			if (!has_collider)
			{
				if (entity.HasComponent<TagComponent>())
				{
					auto& tag = entity.GetComponent<TagComponent>();
					Log::CoreWarn("Entity '{}' has RigidBody but no Collider component", tag.Tag);
				}
				continue;
			}
			
			// Create physics body
			rb.runtime_body = m_physics_world->CreateBody(desc);
		}
	}

	void Scene::SyncTransformsToPhysics()
	{
		auto view = m_registry.view<RigidBodyComponent, TransformComponent>();
		
		for (auto entity_handle : view)
		{
			auto& rb = view.get<RigidBodyComponent>(entity_handle);
			auto& transform = view.get<TransformComponent>(entity_handle);
			
			// Only sync kinematic bodies (dynamic bodies are controlled by physics)
			if (rb.is_kinematic && rb.runtime_body)
			{
				rb.runtime_body->SetPosition(transform.Translation);
				rb.runtime_body->SetRotation(glm::quat(glm::radians(transform.Rotation)));
			}
		}
	}

	void Scene::SyncTransformsFromPhysics()
	{
		auto view = m_registry.view<RigidBodyComponent, TransformComponent>();
		
		for (auto entity_handle : view)
		{
			auto& rb = view.get<RigidBodyComponent>(entity_handle);
			auto& transform = view.get<TransformComponent>(entity_handle);
			
			// Only sync dynamic bodies (static/kinematic don't move via physics)
			if (rb.body_type == BodyType::Dynamic && rb.runtime_body)
			{
				transform.Translation = rb.runtime_body->GetPosition();
				glm::quat rotation = rb.runtime_body->GetRotation();
				transform.Rotation = glm::degrees(glm::eulerAngles(rotation));
			}
		}
	}
}