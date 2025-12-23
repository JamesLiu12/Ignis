#pragma once

#include "Ignis/Editor/EditorPanel.h"

namespace ignis {

	/// <summary>
	/// Base class for debug panels (engine stats, physics debug, profiler, etc.)
	/// Inherits from EditorPanel to integrate with the editor panel system.
	/// 
	/// Note: This class exists for backwards compatibility and semantic clarity.
	/// Debug panels are a specific category of editor panels focused on
	/// engine diagnostics and performance monitoring.
	/// </summary>
	class DebugPanel : public EditorPanel
	{
	public:
		virtual ~DebugPanel() = default;

		// Inherited from EditorPanel:
		// - OnImGuiRender(bool& isOpen)
		// - OnEvent(EventBase& event)
		// - SetSceneContext(Scene* scene)
		// - GetName()
		// - GetID()
	};

}
