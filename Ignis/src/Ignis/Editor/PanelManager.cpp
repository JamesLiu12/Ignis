#include "PanelManager.h"

namespace ignis {

	void PanelManager::OnImGuiRender()
	{
		for (auto& panel_data : m_panels)
		{
			if (panel_data.IsOpen && panel_data.Panel)
			{
				panel_data.Panel->OnImGuiRender();
			}
		}
	}

	void PanelManager::OnEvent(EventBase& e)
	{
		for (auto& panel_data : m_panels)
		{
			if (panel_data.Panel)
				panel_data.Panel->OnEvent(e);
		}
	}

	void PanelManager::SetSceneContext(class Scene* scene)
	{
		for (auto& panel_data : m_panels)
		{
			if (panel_data.Panel)
				panel_data.Panel->SetSceneContext(scene);
		}
	}

} // namespace ignis