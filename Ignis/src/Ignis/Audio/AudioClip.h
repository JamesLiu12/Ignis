#pragma once
#include "Ignis/Asset/Asset.h"
#include <string>

namespace ignis
{
	class AudioClip : public Asset
	{
	public:
		AssetType   GetAssetType() const override { return AssetType::AudioClip; }
		const std::string& GetFilePath()  const { return m_filepath; }
		bool               IsStreaming()  const { return m_stream; }

	private:
		std::string m_filepath;
		bool        m_stream = false;

		friend class AudioImporter;
	};
}