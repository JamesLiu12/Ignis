#include "StreamReader.h"
#include "Ignis/Core/Log.h"

namespace ignis
{
	void StreamReader::ReadBuffer(void* destination, size_t size)
	{
		bool success = ReadData((char*)destination, size);
		if (!success)
		{
			Log::CoreError("Failed to read buffer from stream");
		}
	}

	std::string StreamReader::ReadString()
	{
		uint32_t size;
		ReadRaw<uint32_t>(size);

		std::string result;
		result.resize(size);
		ReadData(result.data(), size);

		return result;
	}
}
