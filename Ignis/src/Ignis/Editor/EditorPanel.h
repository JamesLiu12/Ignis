#pragma once

#include "Ignis/Core/Events/Event.h"
#include <string_view>

namespace ignis {

	// Base class for all editor panels (console, hierarchy, inspector, lighting, etc.)
	// Provides a common interface for panel management and rendering.
	class EditorPanel
	{
	public:
		virtual ~EditorPanel() = default;

		// Called every frame to render the panel's ImGui content.
		// Panel visibility is managed by PanelManager.
		virtual void OnImGuiRender() = 0;

		// Called when an event occurs. Override to handle panel-specific events.
		virtual void OnEvent(EventBase& event) {}

		// Called when the active scene changes. Override if panel needs scene context.
		virtual void SetSceneContext(class Scene* scene) {}

		// Get the panel's display name (for menu items, window titles, etc.)
		virtual std::string_view GetName() const = 0;

		// Get the panel's unique identifier (for serialization, lookups, etc.)
		virtual std::string_view GetID() const { return GetName(); }
	};

} // namespace ignis