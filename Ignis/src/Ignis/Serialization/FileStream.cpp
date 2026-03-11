#include "FileStream.h"
#include "Ignis/Core/Log.h"

namespace ignis
{
	FileStreamWriter::FileStreamWriter(const std::filesystem::path& path)
		: m_path(path)
	{
		std::filesystem::create_directories(path.parent_path());
		m_stream.open(path, std::ios::binary | std::ios::out);
		
		if (!m_stream.is_open())
		{
			Log::CoreError("Failed to open file for writing: {}", path.string());
		}
	}

	FileStreamWriter::~FileStreamWriter()
	{
		if (m_stream.is_open())
			m_stream.close();
	}

	bool FileStreamWriter::WriteData(const char* data, size_t size)
	{
		m_stream.write(data, size);
		return m_stream.good();
	}

	FileStreamReader::FileStreamReader(const std::filesystem::path& path)
		: m_path(path)
	{
		m_stream.open(path, std::ios::binary | std::ios::in);
		
		if (!m_stream.is_open())
		{
			Log::CoreError("Failed to open file for reading: {}", path.string());
		}
	}

	FileStreamReader::~FileStreamReader()
	{
		if (m_stream.is_open())
			m_stream.close();
	}

	bool FileStreamReader::ReadData(char* destination, size_t size)
	{
		m_stream.read(destination, size);
		return m_stream.good();
	}
}
