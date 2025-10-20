#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

namespace ignis {

    class VFS
    {
    public:
        static void Init();
        static void Shutdown();

        // Mount point management
        static void Mount(const std::string& protocol, const std::filesystem::path& physical_path, int priority = 0);
        static void Unmount(const std::string& protocol);
        static bool IsMounted(const std::string& protocol);

        // Path resolution
        static std::filesystem::path Resolve(const std::string& virtual_path);
        static bool Exists(const std::string& virtual_path);

        // File operations through VFS
        static std::vector<uint8_t> ReadBinary(const std::string& virtual_path);
        static std::string ReadText(const std::string& virtual_path);
        static bool Write(const std::string& virtual_path, const std::vector<uint8_t>& data);
        static bool WriteText(const std::string& virtual_path, const std::string& text);

        // Directory operations
        static std::vector<std::string> ListFiles(const std::string& virtual_directory, const std::string& filter = "*");

        // Debugging
        static void PrintMountPoints();

    private:
        struct MountPoint
        {
            std::string protocol;
            std::filesystem::path physical_path;
            int priority;

            bool operator<(const MountPoint& other) const
            {
                return priority > other.priority; // Higher priority first
            }
        };

        static std::unordered_map<std::string, MountPoint> s_mount_points;
        static bool s_initialized;

        static std::pair<std::string, std::string> ParseVirtualPath(const std::string& virtual_path);
    };

}