#pragma once

#include "File.h"

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
        static std::string ParentPath(const std::string& virtual_path);
        static std::string ConcatPath(const std::string& path1, const std::string& path2);

        // File operations through VFS
        static File Open(const std::string& virtual_path);

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