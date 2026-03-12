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

				// Position unparent drop zone at absolute bottom of panel
				// Check if we're currently dragging an entity
				bool is_dragging = ImGui::GetDragDropPayload() != nullptr;
				
				if (is_dragging)
				{
					// Calculate position to place drop zone at bottom
					float drop_zone_height = 25.0f;
					float available_height = ImGui::GetContentRegionAvail().y;
					
					// Add spacing to push drop zone to bottom
					if (available_height > drop_zone_height + 5.0f)
					{
						ImGui::Dummy(ImVec2(0.0f, available_height - drop_zone_height - 5.0f));
					}
					
					ImGui::Separator();
					
					// Show unparent drop zone with light gray color
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.3f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 0.7f));
					
					ImGui::Button("Drop here to unparent entity", ImVec2(-1, drop_zone_height));
					
					ImGui::PopStyleColor(3);
					
					// Drop target on the button
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_REPARENT"))
						{
							UUID dragged_id = *(UUID*)payload->Data;
							Entity dragged = m_scene->GetEntityByID(dragged_id);
							
							if (dragged.IsValid())
							{
								Log::CoreInfo("SceneHierarchy: Unparenting '{}'",
									dragged.GetComponent<TagComponent>().Tag);
								dragged.Unparent();
							}
						}
						ImGui::EndDragDropTarget();
					}
				}

				// Click on blank space to deselect and exit rename mode
				if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
				{
					m_selected_entity = {};
					m_renaming_entity = {};
					if (m_properties_panel)
					{
						m_properties_panel->SetSelectedEntity({});
					}
				}

				// Right-click on blank space to create root entity
				if (ImGui::BeginPopupContextWindow("SceneHierarchyContextMenu", 
					ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
				{
					// Paste entity option (disabled if no entity is copied)
					bool has_copied = m_copied_entity.IsValid();
					#ifdef __APPLE__
					if (ImGui::MenuItem("Paste Entity", "Cmd+V", false, has_copied))
					#else
					if (ImGui::MenuItem("Paste Entity", "Ctrl+V", false, has_copied))
					#endif
					{
						Entity pasted = m_scene->DuplicateEntity(m_copied_entity);
						// Paste as root-level entity (no parent) - keep original position
						m_selected_entity = pasted;
						if (m_properties_panel)
						{
							m_properties_panel->SetSelectedEntity(pasted);
						}
						Log::CoreInfo("SceneHierarchy: Pasted entity as root-level entity");
					}
					
					ImGui::Separator();
					
					DrawEntityCreateMenu();  // No parent = root entity
					ImGui::EndPopup();
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
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
		
		// Get entity name from TagComponent
		std::string name = "Unnamed Entity";
		if (entity.HasComponent<TagComponent>())
		{
			name = entity.GetComponent<TagComponent>().Tag;
		}

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		
		// Highlight if selected
		if (m_selected_entity && m_selected_entity == entity)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		// Check if entity has children
		std::vector<Entity> children = entity.GetChildren();
		if (children.empty())
		{
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		// Check if this entity is being renamed
		bool is_renaming = m_renaming_entity && m_renaming_entity == entity;
		
		bool opened = false;
		if (is_renaming)
		{
			// Show tree node with input field for renaming
			opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "");
			ImGui::SameLine();
			
			ImGui::SetNextItemWidth(-1);
			
			// Set focus before InputText
			ImGui::SetKeyboardFocusHere();
			
			if (ImGui::InputText("##rename", m_rename_buffer, sizeof(m_rename_buffer), 
				ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			{
				// Apply rename on Enter
				if (strlen(m_rename_buffer) > 0)
				{
					entity.GetComponent<TagComponent>().Tag = m_rename_buffer;
				}
				m_renaming_entity = {};
			}
			
			// Exit rename mode on Escape or when input is deactivated (clicked outside)
			if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsItemDeactivated())
			{
				m_renaming_entity = {};
			}
		}
		else
		{
			opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", name.c_str());
		}

		// Handle selection
		if (ImGui::IsItemClicked())
		{
			// Store as shared_ptr to keep entity alive
			m_selected_entity = entity;
			
			// Update properties panel
			if (m_properties_panel)
			{
				m_properties_panel->SetSelectedEntity(m_selected_entity);
				Log::CoreInfo("SceneHierarchy: Selected entity '{}'", name);
			}
		}

		// Tree node drag source for reparenting
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			UUID entity_id = entity.GetID();
			ImGui::SetDragDropPayload("ENTITY_REPARENT", &entity_id, sizeof(UUID));
			ImGui::Text("Reparent: %s", name.c_str());
			ImGui::EndDragDropSource();
		}

		// Drop target to make dragged entity a child
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_REPARENT"))
			{
				UUID dragged_id = *(UUID*)payload->Data;
				Entity dragged = m_scene->GetEntityByID(dragged_id);
				
				// Prevent circular parenting
				bool is_circular = false;
				Entity check = entity;
				while (check.IsValid())
				{
					if (check == dragged)
					{
						is_circular = true;
						break;
					}
					check = check.GetParent();
				}
				
				if (!is_circular && dragged.IsValid() && dragged != entity)
				{
					Log::CoreInfo("SceneHierarchy: Reparenting '{}' to '{}'",
						dragged.GetComponent<TagComponent>().Tag, name);
					dragged.SetParent(entity);
				}
			}
			ImGui::EndDragDropTarget();
		}

		// Right-click context menu for entity
		if (ImGui::BeginPopupContextItem())
		{
			// Rename entity option
			if (ImGui::MenuItem("Rename Entity"))
			{
				m_renaming_entity = entity;
				strncpy(m_rename_buffer, name.c_str(), sizeof(m_rename_buffer) - 1);
				m_rename_buffer[sizeof(m_rename_buffer) - 1] = '\0';
			}
			
			ImGui::Separator();
			
			// Copy entity option
			#ifdef __APPLE__
				if (ImGui::MenuItem("Copy Entity", "Cmd+C"))
			#else
				if (ImGui::MenuItem("Copy Entity", "Ctrl+C"))
			#endif
			{
				m_copied_entity = entity;
				Log::CoreInfo("SceneHierarchy: Copied entity '{}'", name);
			}
			
			// Paste entity option (disabled if no entity is copied)
			bool has_copied = m_copied_entity.IsValid();
			#ifdef __APPLE__
				if (ImGui::MenuItem("Paste Entity", "Cmd+V", false, has_copied))
			#else
				if (ImGui::MenuItem("Paste Entity", "Ctrl+V", false, has_copied))
			#endif
			{
				Entity pasted = m_scene->DuplicateEntity(m_copied_entity);
				pasted.SetParent(entity);
				
				// Reset position to (0,0,0) to place at parent's location
				// Keep original rotation and scale to preserve appearance
				if (pasted.HasComponent<TransformComponent>())
				{
					auto& transform = pasted.GetComponent<TransformComponent>();
					transform.Translation = glm::vec3(0.0f, 0.0f, 0.0f);
				}
				
				m_selected_entity = pasted;
				if (m_properties_panel)
				{
					m_properties_panel->SetSelectedEntity(pasted);
				}
				
				Log::CoreInfo("SceneHierarchy: Pasted entity as child of '{}'", name);
			}
			
			ImGui::Separator();
			
			// Create child entity option
			if (ImGui::MenuItem("Create Child Entity"))
			{
				Entity new_child = m_scene->CreateEntity(entity, "Entity");
				
				// Select the newly created child
				m_selected_entity = new_child;
				if (m_properties_panel)
				{
					m_properties_panel->SetSelectedEntity(m_selected_entity);
				}
				
				Log::CoreInfo("SceneHierarchy: Created child entity '{}' under '{}'", 
							  new_child.GetComponent<TagComponent>().Tag,
							  name);
			}

			ImGui::Separator();
			
			// Delete entity option
			if (ImGui::MenuItem("Delete Entity"))
			{
				// Clear selection if deleting selected entity
				if (m_selected_entity && m_selected_entity == entity)
				{
					m_selected_entity = {};
					if (m_properties_panel)
					{
						m_properties_panel->SetSelectedEntity({});
					}
				}
				
				// Delete the entity (and its children)
				m_scene->DestroyEntity(entity);
				
				Log::CoreInfo("SceneHierarchy: Deleted entity '{}'", name);
				
				// Close popup and tree node before returning
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				
				// CRITICAL: Close tree node if it was opened to prevent ImGui crash
				if (opened && !children.empty())
				{
					ImGui::TreePop();
				}
				
				return;
			}
			
			ImGui::EndPopup();
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
		
		// Restore spacing
		ImGui::PopStyleVar();
	}

	void SceneHierarchyPanel::DrawEntityCreateMenu(Entity parent)
	{
		if (ImGui::MenuItem("Create Empty Entity"))
		{
			Entity new_entity;
			if (parent.IsValid())
			{
				new_entity = m_scene->CreateEntity(parent, "Entity");
			}
			else
			{
				new_entity = m_scene->CreateEntity("Entity");
			}
			
			// Select the newly created entity
			m_selected_entity = new_entity;
			if (m_properties_panel)
			{
				m_properties_panel->SetSelectedEntity(m_selected_entity);
			}
			
			Log::CoreInfo("SceneHierarchy: Created entity '{}'", 
						  new_entity.GetComponent<TagComponent>().Tag);
		}
	}

	void SceneHierarchyPanel::CopySelectedEntity()
	{
		if (!m_selected_entity.IsValid())
		{
			Log::CoreWarn("SceneHierarchy: No entity selected to copy");
			return;
		}

		m_copied_entity = m_selected_entity;
		std::string name = m_selected_entity.GetComponent<TagComponent>().Tag;
		Log::CoreInfo("SceneHierarchy: Copied entity '{}'", name);
	}

	void SceneHierarchyPanel::PasteEntity()
	{
		if (!m_copied_entity.IsValid())
		{
			Log::CoreWarn("SceneHierarchy: No entity copied to paste");
			return;
		}

		if (!m_scene)
		{
			Log::CoreWarn("SceneHierarchy: No scene available");
			return;
		}

		Entity pasted = m_scene->DuplicateEntity(m_copied_entity);

		// If there's a selected entity, make pasted entity a child of it
		// Otherwise, create as root-level entity
		if (m_selected_entity.IsValid())
		{
			pasted.SetParent(m_selected_entity);
			// Reset position to (0,0,0) to place at parent's location
			// Keep original rotation and scale to preserve appearance
			if (pasted.HasComponent<TransformComponent>())
			{
				auto& transform = pasted.GetComponent<TransformComponent>();
				transform.Translation = glm::vec3(0.0f, 0.0f, 0.0f);
			}
			Log::CoreInfo("SceneHierarchy: Pasted entity as child of '{}'", 
				m_selected_entity.GetComponent<TagComponent>().Tag);
		}
		else
		{
			Log::CoreInfo("SceneHierarchy: Pasted entity as root-level entity");
		}

		// Select the pasted entity
		m_selected_entity = pasted;
		if (m_properties_panel)
		{
			m_properties_panel->SetSelectedEntity(pasted);
		}
	}

} // namespace ignis
