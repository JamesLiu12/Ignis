#pragma once

#include "Ignis/Core/Events/Event.h"

namespace ignis {

	/// <summary>
	/// Base class for all editor panels (console, hierarchy, inspector, lighting, etc.)
	/// Provides a common interface for panel management and rendering.
	/// </summary>
	class EditorPanel
	{
	public:
		virtual ~EditorPanel() = default;

		/// <summary>
		/// Called every frame to render the panel's ImGui content.
		/// </summary>
		/// <param name="isOpen">Reference to bool controlling panel visibility.
		/// Set to false to close the panel.</param>
		virtual void OnImGuiRender(bool& isOpen) = 0;

		/// <summary>
		/// Called when an event occurs. Override to handle panel-specific events.
		/// </summary>
		/// <param name="event">The event to handle</param>
		virtual void OnEvent(EventBase& event) {}

		/// <summary>
		/// Called when the active scene changes. Override if panel needs scene context.
		/// </summary>
		/// <param name="scene">Pointer to the new active scene</param>
		virtual void SetSceneContext(class Scene* scene) {}

		/// <summary>
		/// Get the panel's display name (for menu items, window titles, etc.)
		/// </summary>
		virtual const char* GetName() const = 0;

		/// <summary>
		/// Get the panel's unique identifier (for serialization, lookups, etc.)
		/// By default, returns the same as GetName()
		/// </summary>
		virtual const char* GetID() const { return GetName(); }
	};

} // namespace ignis