#include "main/editor/ScenePanel.h"

#include <cstring>
#include <filesystem>

#include "external/imgui/imgui.h"

#include "main/editor/EditorState.h"
#include "main/editor/FileDialog.h"
#include "main/scenes/LoadScene.h"

namespace Editor {

ScenePanel::ScenePanel(int screenWidth, int screenHeight) 
    : screenWidth(screenWidth), screenHeight(screenHeight) {
    currentDirectory = std::filesystem::absolute("../assets/").string();
}

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
                    EditorState::Get().ClearSelection();
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
        std::string selected = ShowFolderDialog();
        if (!selected.empty()) {
            currentDirectory = selected;
        }
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

    ImGui::End();
}

} // namespace Editor
