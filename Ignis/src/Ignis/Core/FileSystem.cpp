#include "FileSystem.h"
#include "Ignis/Core/Log.h"
#include <fstream>
#include <sstream>

namespace ignis {

    bool FileSystem::Exists(const std::filesystem::path& path)
    {
        return std::filesystem::exists(path);
    }

    bool FileSystem::IsDirectory(const std::filesystem::path& path)
    {
        return std::filesystem::is_directory(path);
    }

    bool FileSystem::IsFile(const std::filesystem::path& path)
    {
        return std::filesystem::is_regular_file(path);
    }

    uint64_t FileSystem::GetFileSize(const std::filesystem::path& path)
    {
        if (!Exists(path) || !IsFile(path))
        {
            Log::CoreError("FileSystem: Cannot get size of non-existent file: {}", path.string());
            return 0;
        }
        return std::filesystem::file_size(path);
    }

    uint64_t FileSystem::GetLastModifiedTime(const std::filesystem::path& path)
    {
        if (!Exists(path))
        {
            Log::CoreError("FileSystem: Cannot get modified time of non-existent file: {}", path.string());
            return 0;
        }
        
        auto ftime = std::filesystem::last_write_time(path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        return std::chrono::duration_cast<std::chrono::seconds>(sctp.time_since_epoch()).count();
    }

    bool FileSystem::CreateDirectory(const std::filesystem::path& path)
    {
        if (Exists(path))
            return true;

        std::error_code ec;
        bool result = std::filesystem::create_directory(path, ec);
        
        if (ec)
        {
            Log::CoreError("FileSystem: Failed to create directory '{}': {}", path.string(), ec.message());
            return false;
        }
        
        Log::CoreTrace("FileSystem: Created directory '{}'", path.string());
        return result;
    }

    bool FileSystem::CreateDirectories(const std::filesystem::path& path)
    {
        if (Exists(path))
            return true;

        std::error_code ec;
        bool result = std::filesystem::create_directories(path, ec);
        
        if (ec)
        {
            Log::CoreError("FileSystem: Failed to create directories '{}': {}", path.string(), ec.message());
            return false;
        }
        
        Log::CoreTrace("FileSystem: Created directories '{}'", path.string());
        return result;
    }

    std::vector<std::filesystem::path> FileSystem::ListDirectory(const std::filesystem::path& path)
    {
        std::vector<std::filesystem::path> files;
        
        if (!Exists(path) || !IsDirectory(path))
        {
            Log::CoreError("FileSystem: Cannot list non-existent or non-directory path: {}", path.string());
            return files;
        }

        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            files.push_back(entry.path());
        }

        return files;
    }

    bool FileSystem::Delete(const std::filesystem::path& path)
    {
        if (!Exists(path))
        {
            Log::CoreWarn("FileSystem: Cannot delete non-existent path: {}", path.string());
            return false;
        }

        std::error_code ec;
        bool result = std::filesystem::remove_all(path, ec);
        
        if (ec)
        {
            Log::CoreError("FileSystem: Failed to delete '{}': {}", path.string(), ec.message());
            return false;
        }
        
        Log::CoreTrace("FileSystem: Deleted '{}'", path.string());
        return result;
    }

    bool FileSystem::Copy(const std::filesystem::path& src, const std::filesystem::path& dst)
    {
        if (!Exists(src))
        {
            Log::CoreError("FileSystem: Cannot copy non-existent file: {}", src.string());
            return false;
        }

        std::error_code ec;
        std::filesystem::copy(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
        
        if (ec)
        {
            Log::CoreError("FileSystem: Failed to copy '{}' to '{}': {}", src.string(), dst.string(), ec.message());
            return false;
        }
        
        Log::CoreTrace("FileSystem: Copied '{}' to '{}'", src.string(), dst.string());
        return true;
    }

    bool FileSystem::Move(const std::filesystem::path& src, const std::filesystem::path& dst)
    {
        if (!Exists(src))
        {
            Log::CoreError("FileSystem: Cannot move non-existent file: {}", src.string());
            return false;
        }

        std::error_code ec;
        std::filesystem::rename(src, dst, ec);
        
        if (ec)
        {
            Log::CoreError("FileSystem: Failed to move '{}' to '{}': {}", src.string(), dst.string(), ec.message());
            return false;
        }
        
        Log::CoreTrace("FileSystem: Moved '{}' to '{}'", src.string(), dst.string());
        return true;
    }

    bool FileSystem::Rename(const std::filesystem::path& old_path, const std::filesystem::path& new_path)
    {
        return Move(old_path, new_path);
    }

    std::string FileSystem::GetExtension(const std::filesystem::path& path)
    {
        return path.extension().string();
    }

    std::string FileSystem::GetFileName(const std::filesystem::path& path)
    {
        return path.filename().string();
    }

    std::string FileSystem::GetFileNameWithoutExtension(const std::filesystem::path& path)
    {
        return path.stem().string();
    }

    std::filesystem::path FileSystem::GetParentPath(const std::filesystem::path& path)
    {
        return path.parent_path();
    }

    std::filesystem::path FileSystem::GetAbsolutePath(const std::filesystem::path& path)
    {
        return std::filesystem::absolute(path);
    }

    std::vector<uint8_t> FileSystem::ReadBinaryFile(const std::filesystem::path& path)
    {
        if (!Exists(path))
        {
            Log::CoreError("FileSystem: Cannot read non-existent file: {}", path.string());
            return {};
        }

        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            Log::CoreError("FileSystem: Failed to open file for reading: {}", path.string());
            return {};
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
        {
            Log::CoreError("FileSystem: Failed to read file: {}", path.string());
            return {};
        }

        Log::CoreTrace("FileSystem: Read {} bytes from '{}'", size, path.string());
        return buffer;
    }

    std::string FileSystem::ReadTextFile(const std::filesystem::path& path)
    {
        if (!Exists(path))
        {
            Log::CoreError("FileSystem: Cannot read non-existent file: {}", path.string());
            return "";
        }

        std::ifstream file(path);
        if (!file.is_open())
        {
            Log::CoreError("FileSystem: Failed to open file for reading: {}", path.string());
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        
        std::string content = buffer.str();
        Log::CoreTrace("FileSystem: Read {} characters from '{}'", content.size(), path.string());
        return content;
    }

    bool FileSystem::WriteBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data)
    {
        auto parent = path.parent_path();
        if (!parent.empty() && !Exists(parent))
        {
            CreateDirectories(parent);
        }

        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            Log::CoreError("FileSystem: Failed to open file for writing: {}", path.string());
            return false;
        }

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        
        if (!file.good())
        {
            Log::CoreError("FileSystem: Failed to write to file: {}", path.string());
            return false;
        }

        Log::CoreTrace("FileSystem: Wrote {} bytes to '{}'", data.size(), path.string());
        return true;
    }

    bool FileSystem::WriteTextFile(const std::filesystem::path& path, const std::string& text)
    {
        auto parent = path.parent_path();
        if (!parent.empty() && !Exists(parent))
        {
            CreateDirectories(parent);
        }

        std::ofstream file(path);
        if (!file.is_open())
        {
            Log::CoreError("FileSystem: Failed to open file for writing: {}", path.string());
            return false;
        }

        file << text;
        
        if (!file.good())
        {
            Log::CoreError("FileSystem: Failed to write to file: {}", path.string());
            return false;
        }

        Log::CoreTrace("FileSystem: Wrote {} characters to '{}'", text.size(), path.string());
        return true;
    }

}