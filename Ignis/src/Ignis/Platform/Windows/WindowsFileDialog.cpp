#include "Ignis/Core/File/FileDialog.h"

#ifdef _WIN32
#include <Windows.h>
#include <commdlg.h>
#include <shlobj.h>

namespace ignis {

std::string FileDialog::OpenFile()
{
    OPENFILENAMEA ofn;
    CHAR szFile[260] = { 0 };
    
    // Initialize OPENFILENAME structure
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = nullptr; // TODO: Get GLFW window handle if needed
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    
    // Set file type filters
    ofn.lpstrFilter = "All Supported Files\0*.obj;*.fbx;*.FBX;*.gltf;*.glb;*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr\0"
                      "3D Models\0*.obj;*.fbx;*.FBX;*.gltf;*.glb\0"
                      "Image Files\0*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr\0"
                      "All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    
    // Set flags
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    
    // Set dialog title
    ofn.lpstrTitle = "Select File";
    
    // Display the Open dialog box
    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        // Convert to absolute path
        std::filesystem::path filePath(ofn.lpstrFile);
        return std::filesystem::absolute(filePath).string();
    }
    
    return "";
}

std::string FileDialog::OpenFile(const std::string& filterName, const std::vector<std::string>& filterExtensions)
{
    OPENFILENAMEA ofn;
    CHAR szFile[260] = { 0 };
    
    // Initialize OPENFILENAME structure
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    
    // Build filter string from extensions - must use static buffer for Windows
    static CHAR filterBuffer[512];
    ZeroMemory(filterBuffer, sizeof(filterBuffer));
    
    if (!filterExtensions.empty())
    {
        // Build filter: "FilterName\0*.ext1;*.ext2\0All Files\0*.*\0"
        size_t offset = 0;
        
        // Add filter name
        strncpy_s(filterBuffer + offset, sizeof(filterBuffer) - offset, filterName.c_str(), _TRUNCATE);
        offset += filterName.length() + 1; // +1 for null terminator
        
        // Add extensions pattern
        for (size_t i = 0; i < filterExtensions.size(); ++i)
        {
            if (i > 0)
            {
                filterBuffer[offset++] = ';';
            }
            std::string pattern = "*." + filterExtensions[i];
            strncpy_s(filterBuffer + offset, sizeof(filterBuffer) - offset, pattern.c_str(), _TRUNCATE);
            offset += pattern.length();
        }
        offset++; // Null terminator after extensions
        
        // Add "All Files" option
        const char* allFiles = "All Files";
        strncpy_s(filterBuffer + offset, sizeof(filterBuffer) - offset, allFiles, _TRUNCATE);
        offset += strlen(allFiles) + 1;
        
        const char* allPattern = "*.*";
        strncpy_s(filterBuffer + offset, sizeof(filterBuffer) - offset, allPattern, _TRUNCATE);
        offset += strlen(allPattern) + 1;
        
        // Final null terminator
        filterBuffer[offset] = '\0';
    }
    else
    {
        // Default: "All Files\0*.*\0\0"
        strcpy_s(filterBuffer, sizeof(filterBuffer), "All Files");
        size_t offset = strlen("All Files") + 1;
        strcpy_s(filterBuffer + offset, sizeof(filterBuffer) - offset, "*.*");
        offset += strlen("*.*") + 1;
        filterBuffer[offset] = '\0';
    }
    
    ofn.lpstrFilter = filterBuffer;
    ofn.nFilterIndex = 1;
    
    // Set flags
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    
    // Set dialog title - also needs static buffer
    static CHAR titleBuffer[256];
    std::string title = "Select " + filterName;
    strncpy_s(titleBuffer, sizeof(titleBuffer), title.c_str(), _TRUNCATE);
    ofn.lpstrTitle = titleBuffer;
    
    // Display the Open dialog box
    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        // Convert to absolute path
        std::filesystem::path filePath(ofn.lpstrFile);
        return std::filesystem::absolute(filePath).string();
    }
    
    return "";
}

std::vector<std::string> FileDialog::OpenMultipleFiles()
{
    std::vector<std::string> result;
    
    OPENFILENAMEA ofn;
    CHAR szFile[8192] = { 0 }; // Large buffer for multiple files
    
    // Initialize OPENFILENAME structure
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    
    // Set file type filters
    ofn.lpstrFilter = "All Supported Files\0*.obj;*.fbx;*.FBX;*.gltf;*.glb;*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr\0"
                      "3D Models\0*.obj;*.fbx;*.FBX;*.gltf;*.glb\0"
                      "Image Files\0*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr\0"
                      "All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    
    // Set flags for multiple selection
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOCHANGEDIR;
    
    // Set dialog title
    ofn.lpstrTitle = "Select Files";
    
    // Display the Open dialog box
    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        // Parse the result
        // Format: "directory\0file1\0file2\0...\0\0"
        
        std::string directory = ofn.lpstrFile;
        CHAR* p = ofn.lpstrFile + directory.length() + 1;
        
        if (*p == 0)
        {
            // Single file selected
            std::filesystem::path filePath(directory);
            result.push_back(std::filesystem::absolute(filePath).string());
        }
        else
        {
            // Multiple files selected
            while (*p)
            {
                std::string filename = p;
                std::filesystem::path fullPath = std::filesystem::path(directory) / filename;
                result.push_back(std::filesystem::absolute(fullPath).string());
                p += filename.length() + 1;
            }
        }
    }
    
    return result;
}

std::string FileDialog::SaveFile()
{
    OPENFILENAMEA ofn;
    CHAR szFile[260] = { 0 };
    
    // Initialize OPENFILENAME structure
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    
    // Set file type filters
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    
    // Set flags
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    
    // Set dialog title
    ofn.lpstrTitle = "Save File";
    
    // Display the Save dialog box
    if (GetSaveFileNameA(&ofn) == TRUE)
    {
        // Convert to absolute path
        std::filesystem::path filePath(ofn.lpstrFile);
        return std::filesystem::absolute(filePath).string();
    }
    
    return "";
}

std::string FileDialog::OpenFolder()
{
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = "Select Folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != nullptr)
    {
        CHAR path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path))
        {
            CoTaskMemFree(pidl);
            std::filesystem::path folderPath(path);
            return std::filesystem::absolute(folderPath).string();
        }
        CoTaskMemFree(pidl);
    }
    
    return "";
}

} // namespace ignis

#endif // _WIN32