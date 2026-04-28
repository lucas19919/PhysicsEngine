#include <shobjidl.h>
#include <string>
#include <windows.h>

#ifdef _WIN32
#define NOMINMAX

namespace Editor {

std::string ShowFileDialog(bool save) {
    std::string result = "";
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileDialog *pDialog;
        const CLSID& clsid = save ? CLSID_FileSaveDialog : CLSID_FileOpenDialog;
        const IID& iid = save ? IID_IFileSaveDialog : IID_IFileOpenDialog;
        
        hr = CoCreateInstance(clsid, NULL, CLSCTX_ALL, iid, reinterpret_cast<void**>(&pDialog));
        if (SUCCEEDED(hr)) {
            COMDLG_FILTERSPEC rgSpec[] = { {L"JSON Files", L"*.json"}, {L"All Files", L"*.*"} };
            pDialog->SetFileTypes(2, rgSpec);
            pDialog->SetDefaultExtension(L"json");
            
            hr = pDialog->Show(NULL);
            if (SUCCEEDED(hr)) {
                IShellItem *pItem;
                hr = pDialog->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr)) {
                        int size_needed = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, NULL, 0, NULL, NULL);
                        result.resize(size_needed - 1);
                        WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, &result[0], size_needed, NULL, NULL);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pDialog->Release();
        }
        CoUninitialize();
    }
    return result;
}

std::string ShowFolderDialog() {
    std::string result = "";
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileOpenDialog *pFileOpen;
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
        if (SUCCEEDED(hr)) {
            FILEOPENDIALOGOPTIONS dwOptions;
            pFileOpen->GetOptions(&dwOptions);
            pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
            hr = pFileOpen->Show(NULL);
            if (SUCCEEDED(hr)) {
                IShellItem *pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr)) {
                        int size_needed = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, NULL, 0, NULL, NULL);
                        result.resize(size_needed - 1);
                        WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, &result[0], size_needed, NULL, NULL);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
    return result;
}

} // namespace Editor
#else
namespace Editor {
    std::string ShowFileDialog(bool save) { return ""; }
    std::string ShowFolderDialog() { return ""; }
}
#endif
