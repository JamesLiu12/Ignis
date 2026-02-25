#pragma once

namespace ignis
{
	class Path
	{
	public:
		Path() = default;
		Path(const std::filesystem::path& path);
		Path(const std::string& path);
		Path(const char* path);

		operator std::filesystem::path() const { return m_path; }
		operator std::string() const { return m_path.string(); }

		std::string string() const { return m_path.string(); }
		Path parent_path() const;
		Path filename() const;
		Path extension() const;
		Path stem() const;

		bool empty() const { return m_path.empty(); }
		bool is_absolute() const { return m_path.is_absolute(); }
		bool is_relative() const { return m_path.is_relative(); }

		Path& operator/=(const Path& other);
		Path operator/(const Path& other) const;
		Path& replace_filename(const Path& replacement);

		bool operator==(const Path& other) const { return m_path == other.m_path; }
		bool operator!=(const Path& other) const { return m_path != other.m_path; }
		bool operator<(const Path& other) const { return m_path < other.m_path; }

		const std::filesystem::path& native() const { return m_path; }

	private:
		std::filesystem::path m_path;

		static std::filesystem::path Normalize(const std::filesystem::path& path);
	};

} // namespace ignis

namespace std
{
	template<>
	struct hash<ignis::Path>
	{
		std::size_t operator()(const ignis::Path& path) const
		{
			return std::hash<std::filesystem::path>{}(path.native());
		}
	};
}
