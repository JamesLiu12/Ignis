#pragma once

#include "Ignis/Core/API.h"

namespace ignis
{
	class IGNIS_API StreamReader
	{
	public:
		virtual ~StreamReader() = default;

		virtual bool IsStreamGood() const = 0;
		virtual uint64_t GetStreamPosition() = 0;
		virtual void SetStreamPosition(uint64_t position) = 0;
		virtual bool ReadData(char* destination, size_t size) = 0;

		operator bool() const { return IsStreamGood(); }

		void ReadBuffer(void* destination, size_t size);
		std::string ReadString();

		template<typename T>
		void ReadRaw(T& value)
		{
			static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
			ReadData((char*)&value, sizeof(T));
		}

		template<typename Key, typename Value>
		void ReadMap(std::unordered_map<Key, Value>& map)
		{
			uint32_t size;
			ReadRaw<uint32_t>(size);
			map.clear();
			map.reserve(size);

			for (uint32_t i = 0; i < size; ++i)
			{
				Key key;
				Value value;

				if constexpr (std::is_trivially_copyable_v<Key>)
					ReadRaw<Key>(key);
				else if constexpr (std::is_same_v<Key, std::string>)
					key = ReadString();

				if constexpr (std::is_trivially_copyable_v<Value>)
					ReadRaw<Value>(value);
				else if constexpr (std::is_same_v<Value, std::string>)
					value = ReadString();

				map[key] = value;
			}
		}

		template<typename T>
		void ReadArray(std::vector<T>& array)
		{
			uint32_t size;
			ReadRaw<uint32_t>(size);
			array.clear();
			array.reserve(size);

			for (uint32_t i = 0; i < size; ++i)
			{
				T element;
				if constexpr (std::is_trivially_copyable_v<T>)
					ReadRaw<T>(element);
				else if constexpr (std::is_same_v<T, std::string>)
					element = ReadString();

				array.push_back(element);
			}
		}
	};
}
