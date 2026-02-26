#pragma once
// Need to include the below two headers although in pch.h
// Because macOSFileDialog.mm is using this file and it is a non-pch file
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
     * @return Selected file path, or empty string if cancelled
     */
    static std::string OpenFile();
    
    /**
     * @brief Open a file selection dialog with file type filter
     * @param filterName Display name for the filter (e.g., "Ignis Project Files")
     * @param filterExtensions Vector of allowed extensions without dots (e.g., {"igproj"})
     * @return Selected file path, or empty string if cancelled
     */
    static std::string OpenFile(const std::string& filterName, const std::vector<std::string>& filterExtensions);
    
    /**
     * @brief Open a multiple file selection dialog
     * @return Vector of selected file paths, empty if cancelled
     */
    static std::vector<std::string> OpenMultipleFiles();
    
    /**
     * @brief Open a save file dialog
     * @return Selected save path, or empty string if cancelled
     */
    static std::string SaveFile();
    
    /**
     * @brief Open a folder selection dialog
     * @return Selected folder path, or empty string if cancelled
     */
    static std::string OpenFolder();
};

} // namespace ignis