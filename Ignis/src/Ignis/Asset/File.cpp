#include "File.h"
#include "Ignis/Core/FileSystem.h"
#include "Ignis/Core/Log.h"

namespace ignis {

    File::File(const std::filesystem::path& physical_path)
        : m_path(physical_path)
        , m_exists(false)
        , m_is_open(false)
        , m_is_readable(false)
        , m_is_writable(false)
        , m_size(0)
    {
        CheckFileState();
    }

    void File::CheckFileState()
    {
        // Check if file exists
        m_exists = FileSystem::Exists(m_path);

        if (!m_exists)
        {
            // File doesn't exist - check if we can create it (parent directory writable)
            auto parent = m_path.parent_path();
            if (parent.empty())
                parent = std::filesystem::current_path();

            if (!FileSystem::Exists(parent))
            {
                m_last_error = "Parent directory does not exist: " + parent.string();
                m_is_open = false;
                m_is_readable = false;
                m_is_writable = false;
                return;
            }

            // Try to create the file for writing
            std::ofstream create_test(m_path);
            m_is_writable = create_test.good();
            create_test.close();

            if (m_is_writable)
            {
                // File was created successfully, now it exists
                m_exists = true;
                m_is_readable = true;
                m_is_open = true;
                m_size = 0;
                m_last_error.clear();
                Log::CoreTrace("File: Created new file '{}' - Writable: true", m_path.string());
            }
            else
            {
                m_last_error = "Cannot create file (permission denied): " + m_path.string();
                m_is_open = false;
                m_is_readable = false;
            }
            return;
        }

        // File exists - check if it's a regular file
        if (!FileSystem::IsFile(m_path))
        {
            m_last_error = "Path is not a regular file: " + m_path.string();
            m_is_open = false;
            m_is_readable = false;
            m_is_writable = false;
            return;
        }

        // Get file size
        m_size = FileSystem::GetFileSize(m_path);

        // Try to open for reading
        std::ifstream read_test(m_path, std::ios::binary);
        m_is_readable = read_test.good();
        read_test.close();

        // Try to open for writing (append mode to not destroy file)
        std::ofstream write_test(m_path, std::ios::app);
        m_is_writable = write_test.good();
        write_test.close();

        // File is considered "open" if it's at least readable
        m_is_open = m_is_readable;

        if (!m_is_open)
        {
            m_last_error = "Cannot open file (permission denied or file locked): " + m_path.string();
        }
        else
        {
            m_last_error.clear();
        }

        Log::CoreTrace("File: Checked state for '{}' - Open: {}, Readable: {}, Writable: {}", 
            m_path.string(), m_is_open, m_is_readable, m_is_writable);
    }

    std::vector<uint8_t> File::ReadBinary()
    {
        if (!m_is_open || !m_is_readable)
        {
            Log::CoreError("File: Cannot read from unopened or unreadable file: {}", m_path.string());
            return {};
        }

        return FileSystem::ReadBinaryFile(m_path);
    }

    std::string File::ReadText()
    {
        if (!m_is_open || !m_is_readable)
        {
            Log::CoreError("File: Cannot read from unopened or unreadable file: {}", m_path.string());
            return "";
        }

        return FileSystem::ReadTextFile(m_path);
    }

    bool File::WriteBinary(const std::vector<uint8_t>& data)
    {
        if (!m_is_writable)
        {
            Log::CoreError("File: Cannot write to unwritable file: {}", m_path.string());
            return false;
        }

        bool success = FileSystem::WriteBinaryFile(m_path, data);
        
        if (success)
        {
            // Update file state after writing
            CheckFileState();
        }

        return success;
    }

    bool File::WriteText(const std::string& text)
    {
        if (!m_is_writable)
        {
            Log::CoreError("File: Cannot write to unwritable file: {}", m_path.string());
            return false;
        }

        bool success = FileSystem::WriteTextFile(m_path, text);
        
        if (success)
        {
            // Update file state after writing
            CheckFileState();
        }

        return success;
    }

}