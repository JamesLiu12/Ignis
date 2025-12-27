#include "PanelManager.h"

namespace ignis {

	void PanelManager::OnImGuiRender()
	{
		for (auto& panel_data : m_panels)
		{
			if (panel_data.is_open && panel_data.panel)
			{
				panel_data.panel->OnImGuiRender();
			}
		}
	}

	void PanelManager::OnEvent(EventBase& e)
	{
		for (auto& panel_data : m_panels)
		{
			if (panel_data.panel)
				panel_data.panel->OnEvent(e);
		}
	}

	void PanelManager::SetSceneContext(class Scene* scene)
	{
		for (auto& panel_data : m_panels)
		{
			if (panel_data.panel)
				panel_data.panel->SetSceneContext(scene);
		}
	}

} // namespace ignis