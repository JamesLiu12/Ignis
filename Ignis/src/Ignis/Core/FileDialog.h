#pragma once
#include <string>
#include <vector>

namespace ignis {

/**
 * @brief Cross-platform file dialog utility
 * 
 * Provides native file dialogs for opening/saving files.
 * Platform-specific implementations in Platform/ directory.
 */
class FileDialog
{
public:
    /**
     * @brief Open a file selection dialog
     * @param filter File type filter (currently ignored, shows all supported types)
     * @return Selected file path, or empty string if cancelled
     */
    static std::string OpenFile(const char* filter = nullptr);
    
    /**
     * @brief Open a multiple file selection dialog
     * @param filter File type filter (currently ignored)
     * @return Vector of selected file paths, empty if cancelled
     */
    static std::vector<std::string> OpenMultipleFiles(const char* filter = nullptr);
    
    /**
     * @brief Open a save file dialog
     * @param filter File type filter (currently ignored)
     * @return Selected save path, or empty string if cancelled
     */
    static std::string SaveFile(const char* filter = nullptr);
};

} // namespace ignis