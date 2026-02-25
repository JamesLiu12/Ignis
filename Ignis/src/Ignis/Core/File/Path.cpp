#include "Path.h"

namespace ignis
{
	std::filesystem::path Path::Normalize(const std::filesystem::path& path)
	{
		std::string path_str = path.string();
		std::replace(path_str.begin(), path_str.end(), '\\', '/');
		return std::filesystem::path(path_str);
	}

	Path::Path(const std::filesystem::path& path)
		: m_path(Normalize(path))
	{
	}

	Path::Path(const std::string& path)
		: m_path(Normalize(std::filesystem::path(path)))
	{
	}

	Path::Path(const char* path)
		: m_path(Normalize(std::filesystem::path(path)))
	{
	}

	Path Path::parent_path() const
	{
		return Path(m_path.parent_path());
	}

	Path Path::filename() const
	{
		return Path(m_path.filename());
	}

	Path Path::extension() const
	{
		return Path(m_path.extension());
	}

	Path Path::stem() const
	{
		return Path(m_path.stem());
	}

	Path& Path::operator/=(const Path& other)
	{
		m_path /= other.m_path;
		m_path = Normalize(m_path);
		return *this;
	}

	Path Path::operator/(const Path& other) const
	{
		return Path(m_path / other.m_path);
	}

	Path& Path::replace_filename(const Path& replacement)
	{
		m_path.replace_filename(replacement.m_path);
		m_path = Normalize(m_path);
		return *this;
	}

} // namespace ignis
