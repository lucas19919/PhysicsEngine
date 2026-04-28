#include "main/editor/InspectorPanel.h"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <string>

#include "external/imgui/imgui.h"

#include "main/components/Renderer.h"
#include "main/components/RigidBody.h"
#include "main/components/TransformComponent.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Motor.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/editor/EditorState.h"
#include "main/editor/HistoryManager.h"
#include "main/physics/Config.h"
#include "main/scenes/LoadScene.h"
#include "main/scenes/SaveScene.h"

namespace Editor {

void InspectorPanel::OnImGui(World& world) {
    if (!isOpen) return;

    bool anyItemDeactivated = false;

    ImGui::Begin(GetName(), &isOpen);

    std::string selectedGroup = EditorState::Get().GetSelectedGroup();
    const std::vector<size_t>& selectedObjectIDs = EditorState::Get().GetSelectedObjectIDs();
    size_t selectedID = EditorState::Get().GetSelectedID();
    size_t selectedConstraintID = EditorState::Get().GetSelectedConstraintID();

    if (selectedConstraintID != (size_t)-1) {
        Constraint* selectedConstraint = EditorState::Get().GetSelectedConstraint(world);
        if (selectedConstraint) {
            ImGui::TextDisabled("Constraint Inspector");
            ImGui::Separator();
            ImGui::PushID((int)selectedConstraint->GetID() + 10000);
            
            if (ImGui::CollapsingHeader(selectedConstraint->GetName(), ImGuiTreeNodeFlags_DefaultOpen)) {
                if (selectedConstraint->OnInspectorGui()) {
                    anyItemDeactivated = true;
                }
            }

            if (ImGui::Button("Delete Constraint", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                HistoryManager::Get().RecordState(world);
                world.RemoveConstraint(selectedConstraint->GetID());
                EditorState::Get().SetSelectedConstraint((size_t)-1);
                HistoryManager::Get().RecordState(world);
            }

            ImGui::PopID();
        } else {
            EditorState::Get().SetSelectedConstraint((size_t)-1);
        }
    }
    else if (!selectedGroup.empty()) {
        GeneratorDef* genDef = world.GetGenerator(selectedGroup);
        if (genDef) {
            std::string oldName = genDef->name;
            char nameBuf[128];
            strncpy(nameBuf, oldName.c_str(), sizeof(nameBuf));
            if (ImGui::InputText("Name##Gen", nameBuf, sizeof(nameBuf), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::IsItemDeactivatedAfterEdit()) {
                std::string newName = nameBuf;
                if (!newName.empty() && newName != oldName) {
                    HistoryManager::Get().RecordState(world);
                    world.RenameGenerator(oldName, newName);
                    EditorState::Get().SetSelectedGroup(newName);
                    selectedGroup = newName;
                    HistoryManager::Get().RecordState(world);
                }
            }
            ImGui::Separator();
            ImGui::TextDisabled("Generator Settings");
            bool changed = false;
            if (ImGui::DragInt("Rows", &genDef->rows, 1, 1, 100)) changed = true;
            if (ImGui::DragInt("Columns", &genDef->columns, 1, 1, 100)) changed = true;
            if (ImGui::DragFloat2("Start Pos", &genDef->startX, 0.1f)) changed = true;
            if (ImGui::DragFloat2("Spacing", &genDef->spacingX, 0.1f)) changed = true;
            if (ImGui::IsItemDeactivatedAfterEdit()) anyItemDeactivated = true;
            if (changed) world.RegenerateGenerator(selectedGroup);
        }
    }
    else if (!selectedObjectIDs.empty() && selectedObjectIDs.size() > 1) {
        ImGui::Text("%zu objects selected", selectedObjectIDs.size());
        ImGui::Separator();
        if (ImGui::Button("Delete All", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            HistoryManager::Get().RecordState(world);
            for (size_t id : selectedObjectIDs) world.RemoveGameObject(id);
            EditorState::Get().ClearSelection();
            HistoryManager::Get().RecordState(world);
        }
    }
    else if (selectedID != (size_t)-1) {
        GameObject* selected = EditorState::Get().GetSelected(world);
        if (selected) {
            ImGui::PushID((int)selected->GetID() + 1);
            char nameBuf[128];
            strncpy(nameBuf, selected->GetName().c_str(), sizeof(nameBuf));
            if (ImGui::InputText("Name##Obj", nameBuf, sizeof(nameBuf))) selected->SetName(nameBuf);
            if (ImGui::IsItemDeactivatedAfterEdit()) anyItemDeactivated = true;

            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (selected->transform.OnInspectorGui(&world)) {}
                if (ImGui::IsItemDeactivatedAfterEdit()) anyItemDeactivated = true;
            }

            const auto& components = selected->GetComponents();
            Component* componentToRemove = nullptr;
            for (size_t i = 0; i < components.size(); i++) {
                ImGui::PushID((int)i + 500);
                bool headerOpen = ImGui::CollapsingHeader(components[i]->GetName(), ImGuiTreeNodeFlags_DefaultOpen);
                
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete Component")) componentToRemove = components[i].get();
                    ImGui::EndPopup();
                }

                if (headerOpen) {
                    if (components[i]->OnInspectorGui(&world)) {
                        selected->transform.isDirty = true;
                    }
                    if (ImGui::IsItemDeactivatedAfterEdit()) anyItemDeactivated = true;
                }
                ImGui::PopID();
            }
            if (componentToRemove) {
                HistoryManager::Get().RecordState(world);
                selected->RemoveComponent(componentToRemove);
                HistoryManager::Get().RecordState(world);
            }
            ImGui::PopID();
        } else {
            EditorState::Get().SetSelectedObject((size_t)-1);
        }
    } else {
        ImGui::TextDisabled("Select an object to inspect");
    }

    if (anyItemDeactivated) HistoryManager::Get().RecordState(world);
    ImGui::End();
}

} // namespace Editor
