#pragma once

#include "EditorPanel.h"

namespace ignis {

	class EngineStatsPanel : public EditorPanel
	{
	public:
		EngineStatsPanel();
		void OnImGuiRender() override;

		// EditorPanel interface
		std::string_view GetName() const override { return "Engine Statistics"; }
		std::string_view GetID() const override { return "EngineStats"; }

	private:
		void UpdateFrameStats();
		
		float m_frame_time = 0.0f;
		float m_fps = 0.0f;
		std::chrono::steady_clock::time_point m_last_frame_time;
		int m_frame_count = 0;
	};

}