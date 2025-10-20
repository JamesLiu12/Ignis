#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>

namespace ignis {

    class FileSystem
    {
    public:
        // File existence and info
        static bool Exists(const std::filesystem::path& path);
        static bool IsDirectory(const std::filesystem::path& path);
        static bool IsFile(const std::filesystem::path& path);
        static uint64_t GetFileSize(const std::filesystem::path& path);
        static uint64_t GetLastModifiedTime(const std::filesystem::path& path);

        // Directory operations
        static bool CreateDirectory(const std::filesystem::path& path);
        static bool CreateDirectories(const std::filesystem::path& path);
        static std::vector<std::filesystem::path> ListDirectory(const std::filesystem::path& path);

        // File operations
        static bool Delete(const std::filesystem::path& path);
        static bool Copy(const std::filesystem::path& src, const std::filesystem::path& dst);
        static bool Move(const std::filesystem::path& src, const std::filesystem::path& dst);
        static bool Rename(const std::filesystem::path& old_path, const std::filesystem::path& new_path);

        // Path utilities
        static std::string GetExtension(const std::filesystem::path& path);
        static std::string GetFileName(const std::filesystem::path& path);
        static std::string GetFileNameWithoutExtension(const std::filesystem::path& path);
        static std::filesystem::path GetParentPath(const std::filesystem::path& path);
        static std::filesystem::path GetAbsolutePath(const std::filesystem::path& path);

        // File I/O
        static std::vector<uint8_t> ReadBinaryFile(const std::filesystem::path& path);
        static std::string ReadTextFile(const std::filesystem::path& path);
        static bool WriteBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data);
        static bool WriteTextFile(const std::filesystem::path& path, const std::string& text);
    };

}