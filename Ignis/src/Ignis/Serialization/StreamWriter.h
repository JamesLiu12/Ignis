#pragma once

#include "Ignis/Core/API.h"

namespace ignis
{
	class IGNIS_API StreamWriter
	{
	public:
		virtual ~StreamWriter() = default;

		virtual bool IsStreamGood() const = 0;
		virtual uint64_t GetStreamPosition() = 0;
		virtual void SetStreamPosition(uint64_t position) = 0;
		virtual bool WriteData(const char* data, size_t size) = 0;

		operator bool() const { return IsStreamGood(); }

		void WriteBuffer(const void* data, size_t size, bool write_size = true);
		void WriteZero(uint64_t size);
		void WriteString(const std::string& string);

		template<typename T>
		void WriteRaw(const T& type)
		{
			static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
			WriteData((const char*)&type, sizeof(T));
		}

		template<typename Key, typename Value>
		void WriteMap(const std::unordered_map<Key, Value>& map, bool write_size = true)
		{
			if (write_size)
				WriteRaw<uint32_t>((uint32_t)map.size());

			for (const auto& [key, value] : map)
			{
				if constexpr (std::is_trivially_copyable_v<Key>)
					WriteRaw<Key>(key);
				else if constexpr (std::is_same_v<Key, std::string>)
					WriteString(key);

				if constexpr (std::is_trivially_copyable_v<Value>)
					WriteRaw<Value>(value);
				else if constexpr (std::is_same_v<Value, std::string>)
					WriteString(value);
			}
		}

		template<typename T>
		void WriteArray(const std::vector<T>& array, bool write_size = true)
		{
			if (write_size)
				WriteRaw<uint32_t>((uint32_t)array.size());

			for (const auto& element : array)
			{
				if constexpr (std::is_trivially_copyable_v<T>)
					WriteRaw<T>(element);
				else if constexpr (std::is_same_v<T, std::string>)
					WriteString(element);
			}
		}
	};
}
