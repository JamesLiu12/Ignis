#include "StreamWriter.h"
#include "Ignis/Core/Log.h"

namespace ignis
{
	void StreamWriter::WriteBuffer(const void* data, size_t size, bool write_size)
	{
		if (write_size)
			WriteRaw<uint64_t>(size);

		bool success = WriteData((const char*)data, size);
		if (!success)
		{
			Log::CoreError("Failed to write buffer to stream");
		}
	}

	void StreamWriter::WriteZero(uint64_t size)
	{
		const size_t buffer_size = 4096;
		char zero_buffer[buffer_size] = { 0 };

		while (size > 0)
		{
			size_t write_size = size > buffer_size ? buffer_size : size;
			bool success = WriteData(zero_buffer, write_size);
			if (!success)
			{
				Log::CoreError("Failed to write zeros to stream");
				return;
			}
			size -= write_size;
		}
	}

	void StreamWriter::WriteString(const std::string& string)
	{
		WriteRaw<uint32_t>((uint32_t)string.size());
		WriteData(string.data(), string.size());
	}
}
