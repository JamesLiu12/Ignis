#include "Ignis/Core/FileDialog.h"

#ifdef _WIN32
#include <Windows.h>
#include <commdlg.h>
#include <shlobj.h>

namespace ignis {

std::string FileDialog::OpenFile(const char* filter)
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

std::vector<std::string> FileDialog::OpenMultipleFiles(const char* filter)
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

std::string FileDialog::SaveFile(const char* filter)
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

} // namespace ignis

#endif // _WIN32