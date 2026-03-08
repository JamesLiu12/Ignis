#pragma once

#include "Ignis/Core/API.h"

#include <memory>

namespace ignis
{
	class IGNIS_API AudioEngine
	{
	public:
		static AudioEngine& Get();

		bool Init();
		void Shutdown();
		bool IsInitialized() const;

		void* GetNativeHandle();

		AudioEngine(const AudioEngine&) = delete;
		AudioEngine& operator=(const AudioEngine&) = delete;

	private:
		AudioEngine();
		~AudioEngine();

		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};
}