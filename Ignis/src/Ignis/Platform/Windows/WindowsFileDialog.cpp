#include "Ignis/Core/File/FileDialog.h"

#ifdef _WIN32
#include <Windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shobjidl.h>

namespace ignis {


	static std::string WideToUtf8(PCWSTR wstr)
	{
		if (!wstr) return "";
		int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
		if (size <= 1) return "";
		std::string result(size - 1, '\0');
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), size, nullptr, nullptr);
		return result;
	}

	static std::wstring Utf8ToWide(const std::string& str)
	{
		if (str.empty()) return L"";
		int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		if (size <= 1) return L"";
		std::wstring result(size - 1, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, result.data(), size);
		return result;
	}

	struct ComGuard
	{
		bool initialized = false;
		ComGuard()
		{
			HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			initialized = (hr == S_OK || hr == S_FALSE);
		}
		~ComGuard()
		{
			if (initialized)
				CoUninitialize();
		}
	};

	std::string FileDialog::OpenFile()
	{
		ComGuard com;

		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All Supported Files\0*.obj;*.fbx;*.FBX;*.gltf;*.glb;*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr\0"
			"3D Models\0*.obj;*.fbx;*.FBX;*.gltf;*.glb\0"
			"Image Files\0*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr\0"
			"All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		ofn.lpstrTitle = "Select File";

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			std::filesystem::path filePath(ofn.lpstrFile);
			return std::filesystem::absolute(filePath).string();
		}

		return "";
	}

	std::string FileDialog::OpenFile(const std::string& filterName,
		const std::vector<std::string>& filterExtensions)
	{
		ComGuard com;

		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);

		static CHAR filterBuffer[512];
		ZeroMemory(filterBuffer, sizeof(filterBuffer));

		if (!filterExtensions.empty())
		{
			size_t offset = 0;

			strncpy_s(filterBuffer + offset, sizeof(filterBuffer) - offset,
				filterName.c_str(), _TRUNCATE);
			offset += filterName.length() + 1;

			for (size_t i = 0; i < filterExtensions.size(); ++i)
			{
				if (i > 0) filterBuffer[offset++] = ';';
				std::string pattern = "*." + filterExtensions[i];
				strncpy_s(filterBuffer + offset, sizeof(filterBuffer) - offset,
					pattern.c_str(), _TRUNCATE);
				offset += pattern.length();
			}
			offset++;

			const char* allFiles = "All Files";
			const char* allPattern = "*.*";
			strncpy_s(filterBuffer + offset, sizeof(filterBuffer) - offset, allFiles, _TRUNCATE);
			offset += strlen(allFiles) + 1;
			strncpy_s(filterBuffer + offset, sizeof(filterBuffer) - offset, allPattern, _TRUNCATE);
			offset += strlen(allPattern) + 1;
			filterBuffer[offset] = '\0';
		}
		else
		{
			size_t offset = 0;
			strcpy_s(filterBuffer, sizeof(filterBuffer), "All Files");
			offset = strlen("All Files") + 1;
			strcpy_s(filterBuffer + offset, sizeof(filterBuffer) - offset, "*.*");
			offset += strlen("*.*") + 1;
			filterBuffer[offset] = '\0';
		}

		ofn.lpstrFilter = filterBuffer;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		static CHAR titleBuffer[256];
		std::string title = "Select " + filterName;
		strncpy_s(titleBuffer, sizeof(titleBuffer), title.c_str(), _TRUNCATE);
		ofn.lpstrTitle = titleBuffer;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			std::filesystem::path filePath(ofn.lpstrFile);
			return std::filesystem::absolute(filePath).string();
		}

		return "";
	}

	std::vector<std::string> FileDialog::OpenMultipleFiles()
	{
		std::vector<std::string> result;
		ComGuard com;

		OPENFILENAMEA ofn;
		CHAR szFile[8192] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All Supported Files\0*.obj;*.fbx;*.FBX;*.gltf;*.glb;*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr\0"
			"3D Models\0*.obj;*.fbx;*.FBX;*.gltf;*.glb\0"
			"Image Files\0*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr\0"
			"All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
			OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOCHANGEDIR;
		ofn.lpstrTitle = "Select Files";

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			std::string directory = ofn.lpstrFile;
			CHAR* p = ofn.lpstrFile + directory.length() + 1;

			if (*p == '\0')
			{
				result.push_back(std::filesystem::absolute(directory).string());
			}
			else
			{
				while (*p)
				{
					std::string filename = p;
					auto fullPath = std::filesystem::path(directory) / filename;
					result.push_back(std::filesystem::absolute(fullPath).string());
					p += filename.length() + 1;
				}
			}
		}

		return result;
	}

	std::string FileDialog::SaveFile()
	{
		ComGuard com;

		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
		ofn.lpstrTitle = "Save File";

		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			std::filesystem::path filePath(ofn.lpstrFile);
			return std::filesystem::absolute(filePath).string();
		}

		return "";
	}


	std::string FileDialog::OpenFolder()
	{
		ComGuard com;

		std::string result;

		IFileOpenDialog* pDialog = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&pDialog));
		if (FAILED(hr))
			return "";

		DWORD dwOptions = 0;
		if (SUCCEEDED(pDialog->GetOptions(&dwOptions)))
		{
			pDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);
		}

		pDialog->SetTitle(L"Select Folder");

		hr = pDialog->Show(nullptr);
		if (SUCCEEDED(hr))
		{
			IShellItem* pItem = nullptr;
			hr = pDialog->GetResult(&pItem);
			if (SUCCEEDED(hr))
			{
				PWSTR pszPath = nullptr;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
				if (SUCCEEDED(hr))
				{
					std::string path = WideToUtf8(pszPath);
					CoTaskMemFree(pszPath);
					result = std::filesystem::absolute(path).string();
				}
				pItem->Release();
			}
		}

		pDialog->Release();
		return result;
	}

} // namespace ignis

#endif // _WIN32