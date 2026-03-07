#include "AudioSystem.h"
#include <miniaudio.h>
#include <unordered_map>

#include "AudioEngine.h"
#include "AudioClip.h"
#include "Ignis/Scene/Scene.h"
#include "Ignis/Scene/Components.h"
#include "Ignis/Asset/AssetManager.h"

#include <glm/glm.hpp>

namespace ignis
{
	struct AudioSystem::Impl
	{
		std::unordered_map<UUID, std::unique_ptr<ma_sound>> Sounds;
	};

	AudioSystem::AudioSystem(Scene* scene)
		: m_scene(scene), m_impl(std::make_unique<Impl>()) {
	}

	AudioSystem::~AudioSystem() { UninitAll(); }

	void AudioSystem::OnStart()
	{
		if (!AudioEngine::Get().IsInitialized())
		{
			Log::CoreWarn("AudioSystem::OnStart: AudioEngine is not initialized. "
				"Call AudioEngine::Get().Init() at application startup.");
			return;
		}

		auto view = m_scene->GetAllEntitiesWith<AudioSourceComponent, IDComponent>();
		view.each([&](auto /*handle*/, AudioSourceComponent& /*src*/, IDComponent& id)
			{
				InitEntitySound(id.ID);
			});
	}

	void AudioSystem::OnUpdate(float dt)
	{
		if (!AudioEngine::Get().IsInitialized()) return;

		ma_engine* engine = static_cast<ma_engine*>(AudioEngine::Get().GetNativeHandle());

		{
			auto view = m_scene->GetAllEntitiesWith<AudioListenerComponent, TransformComponent>();
			view.each([&](auto entity_handle, AudioListenerComponent& listener, TransformComponent&)
				{
					if (!listener.Primary) return;

					Entity entity(entity_handle, m_scene);
					glm::mat4 world = entity.GetWorldTransform();

					glm::vec3 pos = glm::vec3(world[3]);
					glm::vec3 fwd = glm::normalize(-glm::vec3(world[2]));
					glm::vec3 up = glm::normalize(glm::vec3(world[1]));

					ma_engine_listener_set_position(engine, 0, pos.x, pos.y, pos.z);
					ma_engine_listener_set_direction(engine, 0, fwd.x, fwd.y, fwd.z);
					ma_engine_listener_set_world_up(engine, 0, up.x, up.y, up.z);
				});
		}

		{
			auto view = m_scene->GetAllEntitiesWith<AudioSourceComponent, TransformComponent, IDComponent>();
			view.each([&](auto entity_handle,
				AudioSourceComponent& src,
				TransformComponent& /*tc*/,
				IDComponent& id)
				{
					if (!src.Spatial) return;

					auto it = m_impl->Sounds.find(id.ID);
					if (it == m_impl->Sounds.end()) return;

					Entity entity(entity_handle, m_scene);
					glm::vec3 pos = glm::vec3(entity.GetWorldTransform()[3]);
					ma_sound_set_position(it->second.get(), pos.x, pos.y, pos.z);
				});
		}
	}

	void AudioSystem::OnStop() { UninitAll(); }


	void AudioSystem::InitEntitySound(UUID entity_id)
	{
		ma_engine* engine = static_cast<ma_engine*>(AudioEngine::Get().GetNativeHandle());
		if (!engine) return;

		Entity entity = m_scene->GetEntityByID(entity_id);
		if (!entity.IsValid() || !entity.HasComponent<AudioSourceComponent>()) return;

		const auto& src = entity.GetComponent<AudioSourceComponent>();
		auto        clip = AssetManager::GetAsset<AudioClip>(src.Clip);

		if (!clip)
		{
			Log::CoreWarn("AudioSystem: Entity {} has AudioSourceComponent but clip asset is invalid.",
				entity_id.ToString());
			return;
		}

		auto sound = std::make_unique<ma_sound>();

		ma_uint32 flags = 0;
		if (clip->IsStreaming())
			flags |= MA_SOUND_FLAG_STREAM;

		ma_result result = ma_sound_init_from_file(
			engine, clip->GetFilePath().string().c_str(), flags, nullptr, nullptr, sound.get());

		if (result != MA_SUCCESS)
		{
			Log::CoreError("AudioSystem: Failed to init sound '{}' for entity {} (error {})",
				clip->GetFilePath(), entity_id.ToString(), static_cast<int>(result));
			return;
		}

		// Apply component config
		ma_sound_set_volume(sound.get(), src.Volume);
		ma_sound_set_pitch(sound.get(), src.Pitch);
		ma_sound_set_looping(sound.get(), src.Loop ? MA_TRUE : MA_FALSE);
		ma_sound_set_spatialization_enabled(sound.get(), src.Spatial ? MA_TRUE : MA_FALSE);

		if (src.Spatial)
		{
			ma_sound_set_attenuation_model(sound.get(), ma_attenuation_model_inverse);
			ma_sound_set_min_distance(sound.get(), src.MinDistance);
			ma_sound_set_max_distance(sound.get(), src.MaxDistance);

			// Set initial world position before first update tick
			glm::vec3 pos = glm::vec3(entity.GetWorldTransform()[3]);
			ma_sound_set_position(sound.get(), pos.x, pos.y, pos.z);
		}

		if (src.PlayOnStart)
			ma_sound_start(sound.get());

		m_impl->Sounds.emplace(entity_id, std::move(sound));
		Log::CoreInfo("AudioSystem: Sound ready for entity {} ({})",
			entity_id.ToString(), clip->IsStreaming() ? "streaming" : "in-memory");
	}

	void AudioSystem::UninitAll()
	{
		for (auto& [id, sound] : m_impl->Sounds)
		{
			if (sound)
				ma_sound_uninit(sound.get());
		}
		m_impl->Sounds.clear();
	}


	void AudioSystem::Play(UUID entity_id)
	{
		auto it = m_impl->Sounds.find(entity_id);
		if (it == m_impl->Sounds.end()) return;
		ma_sound_seek_to_pcm_frame(it->second.get(), 0);
		ma_sound_start(it->second.get());
	}

	void AudioSystem::Stop(UUID entity_id)
	{
		auto it = m_impl->Sounds.find(entity_id);
		if (it == m_impl->Sounds.end()) return;
		ma_sound_stop(it->second.get());
		ma_sound_seek_to_pcm_frame(it->second.get(), 0);
	}

	void AudioSystem::Pause(UUID entity_id)
	{
		auto it = m_impl->Sounds.find(entity_id);
		if (it == m_impl->Sounds.end()) return;
		ma_sound_stop(it->second.get());
	}

	void AudioSystem::Resume(UUID entity_id)
	{
		auto it = m_impl->Sounds.find(entity_id);
		if (it == m_impl->Sounds.end()) return;
		ma_sound_start(it->second.get());
	}

	bool AudioSystem::IsPlaying(UUID entity_id) const
	{
		auto it = m_impl->Sounds.find(entity_id);
		if (it == m_impl->Sounds.end()) return false;
		return ma_sound_is_playing(it->second.get()) == MA_TRUE;
	}

	void AudioSystem::SetVolume(UUID entity_id, float volume)
	{
		auto it = m_impl->Sounds.find(entity_id);
		if (it == m_impl->Sounds.end()) return;
		ma_sound_set_volume(it->second.get(), volume);
	}

	void AudioSystem::SetPitch(UUID entity_id, float pitch)
	{
		auto it = m_impl->Sounds.find(entity_id);
		if (it == m_impl->Sounds.end()) return;
		ma_sound_set_pitch(it->second.get(), pitch);
	}
}