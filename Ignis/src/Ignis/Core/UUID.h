#pragma once

#include <uuid.h>

namespace ignis
{
	class UUID {
	public:
		UUID();
		UUID(const uuids::uuid& uuid);

		explicit UUID(const std::string& uuid_string);

		std::string ToString() const;

		bool operator==(const UUID& other) const { return m_uuid == other.m_uuid; }
		bool operator!=(const UUID& other) const { return m_uuid != other.m_uuid; }
		bool operator<(const UUID& other) const { return m_uuid < other.m_uuid; }

		bool IsValid() const;

		uuids::uuid GetRaw() const { return m_uuid; }

		inline static const uuids::uuid InvalidUUID = {};

	private:
		uuids::uuid m_uuid;
	};
}

namespace std
	{
	template<>
	struct hash<ignis::UUID>
	{
		std::size_t operator()(const ignis::UUID& uuid) const
		{
			return std::hash<uuids::uuid>{}(uuid.GetRaw());
		}
	};
}