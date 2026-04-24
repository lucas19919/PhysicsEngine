#include "main/editor/InspectorPanel.h"
#include "external/imgui/imgui.h"
#include "main/editor/EditorState.h"
#include "main/components/TransformComponent.h"

namespace Editor {

void InspectorPanel::OnImGui(World& world) {
    if (!isOpen) return;

    ImGui::Begin(GetName(), &isOpen);

    GameObject* selected = EditorState::Get().GetSelected();
    if (selected) {
        ImGui::Text("ID: %zu", selected->GetID());
        ImGui::Separator();

        // Transform is a special component (inline struct)
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            selected->transform.OnInspectorGui();
        }

        // Iterate through all other components
        for (auto& component : selected->GetComponents()) {
            if (ImGui::CollapsingHeader(component->GetName(), ImGuiTreeNodeFlags_DefaultOpen)) {
                component->OnInspectorGui();
            }
        }
    } else {
        ImGui::TextDisabled("Select an object to inspect");
    }

    ImGui::End();
}

} // namespace Editor
