#include "UUID.h"

namespace ignis
{
	static thread_local std::random_device rd;
	static thread_local std::mt19937 generator(rd());
	static thread_local uuids::uuid_random_generator gen(generator);

	UUID::UUID()
		: m_uuid(gen())
	{

	}

	UUID::UUID(const uuids::uuid& uuid)
		: m_uuid(uuid)
	{

	}

	UUID::UUID(const std::string& uuid_string) {
		auto optional_uuid = uuids::uuid::from_string(uuid_string);
		if (optional_uuid.has_value()) {
			m_uuid = optional_uuid.value();
		}
	}

	std::string UUID::ToString() const {
		return uuids::to_string(m_uuid);
	}

	bool UUID::IsValid() const {
		return !m_uuid.is_nil();
	}
}