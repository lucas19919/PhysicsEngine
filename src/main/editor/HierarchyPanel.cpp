#include "main/editor/HierarchyPanel.h"
#include "external/imgui/imgui.h"
#include "main/editor/EditorState.h"
#include <string>
#include <map>
#include <vector>

namespace Editor {

void HierarchyPanel::OnImGui(World& world) {
    if (!isOpen) return;

    ImGui::Begin(GetName(), &isOpen);

    const auto& objects = world.GetGameObjects();
    GameObject* selected = EditorState::Get().GetSelected();

    // Group objects by groupName
    std::map<std::string, std::vector<GameObject*>> groups;
    std::vector<GameObject*> ungrouped;

    for (const auto& objPtr : objects) {
        GameObject* obj = objPtr.get();
        if (obj->GetGroupName().empty()) {
            ungrouped.push_back(obj);
        } else {
            groups[obj->GetGroupName()].push_back(obj);
        }
    }

    auto renderNode = [&](GameObject* obj) {
        std::string objName = obj->GetName();
        if (objName.empty()) {
            objName = "GameObject " + std::to_string(obj->GetID());
        }
        
        std::string label = objName;
        ImGuiTreeNodeFlags flags = ((selected == obj) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool opened = ImGui::TreeNodeEx((void*)(intptr_t)obj->GetID(), flags, "%s", label.c_str());
        if (ImGui::IsItemClicked()) {
            EditorState::Get().SetSelected(obj);
        }
        if (opened) {
            ImGui::TreePop();
        }
    };

    // Render Groups (Folders)
    for (auto& pair : groups) {
        if (ImGui::TreeNode(pair.first.c_str())) {
            for (GameObject* obj : pair.second) {
                renderNode(obj);
            }
            ImGui::TreePop();
        }
    }

    // Render Ungrouped
    for (GameObject* obj : ungrouped) {
        renderNode(obj);
    }

    ImGui::End();
}

} // namespace Editor
