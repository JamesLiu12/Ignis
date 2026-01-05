#include "SceneHierarchyPanel.h"
#include "PropertiesPanel.h"
#include "Ignis/Scene/Components.h"
#include <imgui.h>

namespace ignis {

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos;
		ImVec2 work_size = viewport->WorkSize;

		float panel_width = 250.0f;
		float console_height = 200.0f;
		
		// Position: left side, height = window height - console height
		ImGui::SetNextWindowPos(ImVec2(work_pos.x, work_pos.y));
		ImGui::SetNextWindowSize(ImVec2(panel_width, work_size.y - console_height));

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

		if (ImGui::Begin("Scene Hierarchy", nullptr, window_flags))
		{
			if (m_scene)
			{
				// Iterate through all entities in the scene
				auto view = m_scene->GetAllEntitiesWith<TagComponent>();
				for (auto entityHandle : view)
				{
					Entity entity = m_scene->GetEntityByHandle(entityHandle);
					
					// Only show entities with light components
					if (entity.HasComponent<DirectionalLightComponent>() ||
					    entity.HasComponent<PointLightComponent>() ||
					    entity.HasComponent<SpotLightComponent>() ||
					    entity.HasComponent<SkyLightComponent>())
					{
						DrawEntityNode(entity);
					}
				}

				// Click on blank space to deselect
				if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				{
					m_selected_entity = nullptr;
					if (m_properties_panel)
					{
						m_properties_panel->SetSelectedEntity(nullptr);
					}
				}
			}
			else
			{
				ImGui::TextDisabled("No scene loaded");
			}
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		// Get entity name from TagComponent
		std::string name = "Unnamed Entity";
		if (entity.HasComponent<TagComponent>())
		{
			name = entity.GetComponent<TagComponent>().Tag;
		}

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		
		// Highlight if selected
		if (m_selected_entity && *m_selected_entity == entity)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		// Use leaf flag since we don't have hierarchy yet
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", name.c_str());

		// Handle selection
		if (ImGui::IsItemClicked())
		{
			// Store as shared_ptr to keep entity alive
			m_selected_entity = std::make_shared<Entity>(entity);
			
			// Update properties panel
			if (m_properties_panel)
			{
				m_properties_panel->SetSelectedEntity(m_selected_entity);
				Log::CoreInfo("SceneHierarchy: Selected entity '{}'", name);
			}
		}
	}

} // namespace ignis
