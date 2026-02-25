#pragma once

namespace ignis {

    class File
    {
    public:
        File(const std::filesystem::path& physical_path);

        // State checking
        bool IsOpen() const { return m_is_open; }
        bool IsReadable() const { return m_is_readable; }
        bool IsWritable() const { return m_is_writable; }
        bool Exists() const { return m_exists; }

        // File information
        uint64_t GetSize() const { return m_size; }
        const std::filesystem::path& GetPath() const { return m_path; }
        const std::string& GetError() const { return m_last_error; }

        // Read operations
        std::vector<uint8_t> ReadBinary();
        std::string ReadText();

        // Write operations
        bool WriteBinary(const std::vector<uint8_t>& data);
        bool WriteText(const std::string& text);

        // Stream access for serialization
        std::ifstream OpenInputStream() const;
        std::ofstream OpenOutputStream(bool append = false) const;

    private:
        std::filesystem::path m_path;
        bool m_exists;
        bool m_is_open;
        bool m_is_readable;
        bool m_is_writable;
        uint64_t m_size;
        std::string m_last_error;

        void CheckFileState();
    };

}
