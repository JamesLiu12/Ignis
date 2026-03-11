#pragma once

#include "Ignis/Core/Application.h"

namespace ignis {

class RuntimeSceneLayer;

class RuntimeApp : public Application
{
public:
	RuntimeApp(const std::string& project_path);
	~RuntimeApp() override;

	void OnUpdate(float dt) override;
	
	RuntimeSceneLayer* GetSceneLayer() { return m_scene_layer; }

private:
	std::string m_project_path;
	RuntimeSceneLayer* m_scene_layer = nullptr;
};

} // namespace ignis
