#include "AudioEngine.h"

#include <miniaudio.h>

namespace ignis
{
	struct AudioEngine::Impl
	{
		ma_engine Engine{};
		bool      Initialized = false;
	};

	AudioEngine::AudioEngine() : m_impl(std::make_unique<Impl>()) {}
	AudioEngine::~AudioEngine() { Shutdown(); }

	AudioEngine& AudioEngine::Get()
	{
		static AudioEngine instance;
		return instance;
	}

	bool AudioEngine::Init()
	{
		if (m_impl->Initialized)
			return true;

		ma_result result = ma_engine_init(nullptr, &m_impl->Engine);
		if (result != MA_SUCCESS)
		{
			Log::CoreError("AudioEngine: Failed to initialize (error {})", static_cast<int>(result));
			return false;
		}

		m_impl->Initialized = true;
		Log::CoreInfo("AudioEngine: Initialized - {} Hz, {} channels",
			ma_engine_get_sample_rate(&m_impl->Engine),
			ma_engine_get_channels(&m_impl->Engine));
		return true;
	}

	void AudioEngine::Shutdown()
	{
		if (!m_impl->Initialized) return;
		ma_engine_uninit(&m_impl->Engine);
		m_impl->Initialized = false;
		// Don't log here as logger may be already shut down
	}

	bool AudioEngine::IsInitialized() const { return m_impl->Initialized; }

	void* AudioEngine::GetNativeHandle()
	{
		return m_impl->Initialized ? &m_impl->Engine : nullptr;
	}
}