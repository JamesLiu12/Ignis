#pragma once

#include "EditorPanel.h"

namespace ignis {

	/// <summary>
	/// Properties panel for displaying and editing object properties
	/// Currently empty - positioned on right side
	/// </summary>
	class PropertiesPanel : public EditorPanel
	{
	public:
		PropertiesPanel() = default;
		~PropertiesPanel() = default;

		// EditorPanel interface
		void OnImGuiRender() override;
		std::string_view GetName() const override { return "Properties"; }
		std::string_view GetID() const override { return "Properties"; }
	};

} // namespace ignis