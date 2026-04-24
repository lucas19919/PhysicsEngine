#include "main/editor/ScenePanel.h"
#include "external/imgui/imgui.h"
#include "main/scenes/LoadScene.h"
#include "main/editor/EditorState.h"
#include <cstring>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <shobjidl.h> 
#include <shlobj.h>
#endif

namespace Editor {

ScenePanel::ScenePanel(int screenWidth, int screenHeight) 
    : screenWidth(screenWidth), screenHeight(screenHeight) {
    currentDirectory = std::filesystem::absolute("../assets/").string();
}

#ifdef _WIN32
std::string OpenFolderDialog() {
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
#endif

void RenderFileTree(const std::filesystem::path& path, World& world, int screenWidth, int screenHeight) {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                if (ImGui::TreeNode(entry.path().filename().string().c_str())) {
                    RenderFileTree(entry.path(), world, screenWidth, screenHeight);
                    ImGui::TreePop();
                }
            } else if (entry.is_regular_file() && entry.path().extension() == ".json") {
                std::string fileName = entry.path().filename().string();
                bool isSelected = (EditorState::Get().GetActiveScenePath() == entry.path().string());
                
                if (ImGui::Selectable(fileName.c_str(), isSelected)) {
                    EditorState::Get().SetSelected(nullptr); // FIX: Clear selection before destroying objects
                    EditorState::Get().SetActiveScenePath(entry.path().string());
                    world.isPaused = true;
                    world.Clear();
                    LoadScene::Load(EditorState::Get().GetActiveScenePath(), world, screenWidth, screenHeight);
                }
            }
        }
    } catch (const std::exception& e) {
        ImGui::TextDisabled("Access Error in: %s", path.filename().string().c_str());
    }
}

void ScenePanel::OnImGui(World& world) {
    if (!isOpen) return;

    ImGui::Begin(GetName(), &isOpen);
    
    ImGui::TextWrapped("Root: %s", currentDirectory.c_str());
    
    if (ImGui::Button("Set Assets Root...")) {
#ifdef _WIN32
        std::string selected = OpenFolderDialog();
        if (!selected.empty()) {
            currentDirectory = selected;
        }
#else
        ImGui::OpenPopup("BrowserNotSupported");
#endif
    }

    ImGui::Separator();

    if (ImGui::BeginChild("FileTreeContent")) {
        if (std::filesystem::exists(currentDirectory) && std::filesystem::is_directory(currentDirectory)) {
            RenderFileTree(currentDirectory, world, screenWidth, screenHeight);
        } else {
            ImGui::TextDisabled("No valid root folder selected.");
        }
        ImGui::EndChild();
    }

    if (ImGui::BeginPopupModal("BrowserNotSupported", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Native folder browser is only supported on Windows.");
        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    ImGui::End();
}

} // namespace Editor
