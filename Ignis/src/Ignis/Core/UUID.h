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

	private:
		uuids::uuid m_uuid;
	};
}