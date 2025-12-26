#include "PanelManager.h"

namespace ignis {

	void PanelManager::OnImGuiRender()
	{
		for (auto& panelData : m_panels)
		{
			if (panelData.IsOpen && panelData.Panel)
			{
				panelData.Panel->OnImGuiRender();
			}
		}
	}

	void PanelManager::OnEvent(EventBase& e)
	{
		for (auto& panelData : m_panels)
		{
			if (panelData.Panel)
				panelData.Panel->OnEvent(e);
		}
	}

	void PanelManager::SetSceneContext(class Scene* scene)
	{
		for (auto& panelData : m_panels)
		{
			if (panelData.Panel)
				panelData.Panel->SetSceneContext(scene);
		}
	}

} // namespace ignis