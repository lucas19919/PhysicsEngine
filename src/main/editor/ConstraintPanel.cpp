#include "main/editor/ConstraintPanel.h"
#include "external/imgui/imgui.h"
#include "main/World.h"
#include "main/editor/EditorState.h"
#include "main/editor/HistoryManager.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/components/constrainttypes/Motor.h"

namespace Editor {

static std::string GetConstraintTypeName(ConstraintType type) {
    switch (type) {
        case DISTANCE: return "Distance";
        case PIN: return "Pin";
        case JOINT: return "Joint";
        case MOTOR: return "Motor";
        default: return "Unknown";
    }
}

void ConstraintPanel::OnImGui(World& world) {
    if (!isOpen) return;

    ImGui::Begin(GetName(), &isOpen);

    const auto& constraints = world.GetConstraints();
    if (constraints.empty()) {
        ImGui::TextDisabled("No constraints in the world.");
    } else {
        if (ImGui::BeginTable("ConstraintsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Objects");
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            int toRemove = -1;
            size_t currentSelectedID = EditorState::Get().GetSelectedConstraintID();

            for (const auto& c : constraints) {
                ImGui::TableNextRow();
                
                bool isSelected = (currentSelectedID == c->GetID());

                ImGui::TableSetColumnIndex(0);
                std::string label = GetConstraintTypeName(c->GetType()) + "##" + std::to_string(c->GetID());
                if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                    EditorState::Get().SetSelectedConstraint(c->GetID());
                }

                ImGui::TableSetColumnIndex(1);
                // List objects involved
                if (c->GetType() == DISTANCE) {
                    auto* dc = static_cast<DistanceConstraint*>(c.get());
                    ImGui::Text("%s <-> %s", dc->anchor->GetName().c_str(), dc->attached->GetName().c_str());
                } else if (c->GetType() == PIN || c->GetType() == JOINT) {
                    ImGui::Text("%zu objects", (c->GetType() == PIN) ? static_cast<PinConstraint*>(c.get())->attachments.size() : static_cast<JointConstraint*>(c.get())->attachments.size());
                } else if (c->GetType() == MOTOR) {
                    auto* mc = static_cast<MotorConstraint*>(c.get());
                    ImGui::Text("%s", mc->rotor->GetName().c_str());
                }

                ImGui::TableSetColumnIndex(2);
                ImGui::PushID((int)c->GetID());
                if (ImGui::SmallButton("Delete")) {
                    toRemove = (int)c->GetID();
                }
                ImGui::PopID();
            }
            ImGui::EndTable();

            if (toRemove != -1) {
                HistoryManager::Get().RecordState(world);
                world.RemoveConstraint(toRemove);
                HistoryManager::Get().RecordState(world);
            }
        }
    }

    ImGui::End();
}

} // namespace Editor
