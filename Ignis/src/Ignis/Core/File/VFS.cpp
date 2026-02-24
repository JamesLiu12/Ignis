#include "VFS.h"
#include "FileSystem.h"
#include "Ignis/Core/Log.h"

namespace ignis {

    std::unordered_map<std::string, VFS::MountPoint> VFS::s_mount_points;
    bool VFS::s_initialized = false;

    void VFS::Init()
    {
        if (s_initialized)
        {
            Log::CoreWarn("VFS: Already initialized!");
            return;
        }

        s_mount_points.clear();
        s_initialized = true;
        
        Log::CoreInfo("VFS: Initialized");
    }

    void VFS::Shutdown()
    {
        if (!s_initialized)
            return;

        s_mount_points.clear();
        s_initialized = false;
        
        Log::CoreInfo("VFS: Shutdown");
    }

    void VFS::Mount(const std::string& protocol, const std::filesystem::path& physical_path, int priority)
    {
        if (!s_initialized)
        {
            Log::CoreError("VFS: Cannot mount '{}' - VFS not initialized!", protocol);
            return;
        }

        auto absolute_path = FileSystem::GetAbsolutePath(physical_path);
        
        if (!FileSystem::Exists(absolute_path))
        {
            Log::CoreWarn("VFS: Mount path does not exist: '{}' -> '{}'", protocol, absolute_path.string());
        }

        MountPoint mount_point;
        mount_point.protocol = protocol;
        mount_point.physical_path = absolute_path;
        mount_point.priority = priority;

        s_mount_points[protocol] = mount_point;
        
        Log::CoreInfo("VFS: Mounted '{}' -> '{}' (priority: {})", protocol, absolute_path.string(), priority);
    }

    void VFS::Unmount(const std::string& protocol)
    {
        if (!s_initialized)
        {
            Log::CoreError("VFS: Cannot unmount - VFS not initialized!");
            return;
        }

        auto it = s_mount_points.find(protocol);
        if (it != s_mount_points.end())
        {
            Log::CoreInfo("VFS: Unmounted '{}'", protocol);
            s_mount_points.erase(it);
        }
        else
        {
            Log::CoreWarn("VFS: Cannot unmount '{}' - not mounted", protocol);
        }
    }

    bool VFS::IsMounted(const std::string& protocol)
    {
        return s_mount_points.find(protocol) != s_mount_points.end();
    }

    std::pair<std::string, std::string> VFS::ParseVirtualPath(const std::string& virtual_path)
    {
        size_t separator = virtual_path.find("://");
        
        if (separator == std::string::npos)
        {
            return {"", virtual_path};
        }

        std::string protocol = virtual_path.substr(0, separator);
        std::string relative_path = virtual_path.substr(separator + 3);
        
        return {protocol, relative_path};
    }

    std::filesystem::path VFS::Resolve(const std::string& virtual_path)
    {
        if (!s_initialized)
        {
            Log::CoreError("VFS: Cannot resolve path - VFS not initialized!");
            return "";
        }

        auto [protocol, relative_path] = ParseVirtualPath(virtual_path);

        if (protocol.empty())
        {
            Log::CoreTrace("VFS: No protocol in path '{}', treating as filesystem path", virtual_path);
            return virtual_path;
        }

        auto it = s_mount_points.find(protocol);
        if (it == s_mount_points.end())
        {
            Log::CoreError("VFS: Unknown protocol '{}' in path '{}'", protocol, virtual_path);
            return "";
        }

        std::filesystem::path physical_path = it->second.physical_path / relative_path;
        Log::CoreTrace("VFS: Resolved '{}' -> '{}'", virtual_path, physical_path.string());
        
        return physical_path;
    }

    bool VFS::Exists(const std::string& virtual_path)
    {
        auto physical_path = Resolve(virtual_path);
        if (physical_path.empty())
            return false;

        return FileSystem::Exists(physical_path);
    }

    std::string VFS::ParentPath(const std::string& virtual_path)
    {
		if (virtual_path.empty()) {
			return "";
		}

		size_t end_pos = virtual_path.length() - 1;

		while (end_pos < virtual_path.length() &&
			(virtual_path[end_pos] == '/' || virtual_path[end_pos] == '\\')) {
			if (end_pos == 0) {
				return "";
			}
			end_pos--;
		}

		while (end_pos < virtual_path.length()) {
			if (virtual_path[end_pos] == '/' || virtual_path[end_pos] == '\\') {
				return virtual_path.substr(0, end_pos + 1);
			}

			if (end_pos == 0) {
				break;
			}
			end_pos--;
		}

		return "";
    }

    std::string VFS::ConcatPath(const std::string& path1, const std::string& path2)
    {
		if (path1.empty()) return path2;
		if (path2.empty()) return path1;

		char last_char = path1.back();
		bool p1_ends_slash = (last_char == '/' || last_char == '\\');

		char first_char = path2.front();
		bool p2_starts_slash = (first_char == '/' || first_char == '\\');

		if (p1_ends_slash && p2_starts_slash) {
			return path1 + path2.substr(1);
		}
		else if (!p1_ends_slash && !p2_starts_slash) {
			char separator = '/';
			if (path1.find('\\') != std::string::npos) {
				separator = '\\';
			}
			return path1 + separator + path2;
		}
		else {
			return path1 + path2;
		}
    }

    File VFS::Open(const std::string& virtual_path)
    {
        auto physical_path = Resolve(virtual_path);
        if (physical_path.empty())
        {
            Log::CoreError("VFS: Failed to resolve path: {}", virtual_path);
            // Return a File object with an invalid path
            return File("");
        }

        Log::CoreTrace("VFS: Opening file '{}'", virtual_path);
        return File(physical_path);
    }

    std::vector<std::string> VFS::ListFiles(const std::string& virtual_directory, const std::string& filter)
    {
        auto physical_path = Resolve(virtual_directory);
        if (physical_path.empty())
        {
            Log::CoreError("VFS: Failed to resolve directory: {}", virtual_directory);
            return {};
        }

        auto files = FileSystem::ListDirectory(physical_path);
        
        std::vector<std::string> result;
        for (const auto& file : files)
        {
            result.push_back(file.filename().string());
        }

        return result;
    }

    void VFS::PrintMountPoints()
    {
        Log::CoreInfo("VFS: Mount Points ({}):", s_mount_points.size());
        
        std::vector<MountPoint> sorted_mounts;
        for (const auto& [protocol, mount] : s_mount_points)
        {
            sorted_mounts.push_back(mount);
        }
        std::sort(sorted_mounts.begin(), sorted_mounts.end());

        for (const auto& mount : sorted_mounts)
        {
            Log::CoreInfo("  '{}' -> '{}' (priority: {})", 
                mount.protocol, mount.physical_path.string(), mount.priority);
        }
    }

}