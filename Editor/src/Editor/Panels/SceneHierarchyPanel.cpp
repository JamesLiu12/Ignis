#include "Editor/Panels/SceneHierarchyPanel.h"
#include "Editor/Panels/PropertiesPanel.h"
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
				auto view = m_scene->GetAllEntitiesWith<TagComponent, RelationshipComponent>();
				for (auto entity_handle : view)
				{
					Entity entity = m_scene->GetEntityByHandle(entity_handle);
					
					// Only show root entities (entities with no parent)
					if (entity.GetParentID() == UUID::Invalid)
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

		// Check if entity has children
		std::vector<Entity> children = entity.GetChildren();
		if (children.empty())
		{
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

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

		// Recursively render children if tree node is open
		if (opened && !children.empty())
		{
			for (Entity child : children)
			{
				DrawEntityNode(child);
			}
			ImGui::TreePop();
		}
	}

} // namespace ignis
