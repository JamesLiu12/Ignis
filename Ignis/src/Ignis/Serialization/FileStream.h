#pragma once

#include "StreamWriter.h"
#include "StreamReader.h"

namespace ignis
{
	class IGNIS_API FileStreamWriter : public StreamWriter
	{
	public:
		FileStreamWriter(const std::filesystem::path& path);
		FileStreamWriter(const FileStreamWriter&) = delete;
		virtual ~FileStreamWriter();

		bool IsStreamGood() const final { return m_stream.good(); }
		uint64_t GetStreamPosition() final { return m_stream.tellp(); }
		void SetStreamPosition(uint64_t position) final { m_stream.seekp(position); }
		bool WriteData(const char* data, size_t size) final;

	private:
		std::filesystem::path m_path;
		std::ofstream m_stream;
	};

	class IGNIS_API FileStreamReader : public StreamReader
	{
	public:
		FileStreamReader(const std::filesystem::path& path);
		FileStreamReader(const FileStreamReader&) = delete;
		~FileStreamReader();

		bool IsStreamGood() const final { return m_stream.good(); }
		uint64_t GetStreamPosition() override { return m_stream.tellg(); }
		void SetStreamPosition(uint64_t position) override { m_stream.seekg(position); }
		bool ReadData(char* destination, size_t size) override;

	private:
		std::filesystem::path m_path;
		std::ifstream m_stream;
	};
}
