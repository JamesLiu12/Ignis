#pragma once

#include "Ignis/Core/API.h"
#include "Ignis/Core/UUID.h"

#include <memory>

namespace ignis
{
	class Scene;

	class IGNIS_API AudioSystem
	{
	public:
		explicit AudioSystem(Scene* scene);
		~AudioSystem();

		void OnStart();
		void OnUpdate(float dt);
		void OnStop();

		void Play(UUID entity_id);
		void Stop(UUID entity_id);
		void Pause(UUID entity_id);
		void Resume(UUID entity_id);

		bool  IsPlaying(UUID entity_id) const;
		void  SetVolume(UUID entity_id, float volume);
		void  SetPitch(UUID entity_id, float pitch);

	private:
		void InitEntitySound(UUID entity_id);
		void UninitAll();

		Scene* m_scene = nullptr;

		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};
}