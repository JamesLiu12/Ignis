#pragma once

#include "DebugPanel.h"
#include "pch.h"

namespace ignis {

	class EngineStatsPanel : public DebugPanel
	{
	public:
		EngineStatsPanel();
		void OnImGuiRender(bool& is_open) override;

	private:
		void UpdateFrameStats();
		
		float m_frame_time = 0.0f;
		float m_fps = 0.0f;
		std::chrono::steady_clock::time_point m_last_frame_time;
		int m_frame_count = 0;
	};

}