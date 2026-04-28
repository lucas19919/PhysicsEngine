#include "main/components/Collider.h"

#include <string>
#include <vector>

#include "external/imgui/imgui.h"

#include "main/GameObject.h"
#include "main/World.h"

bool Collider::OnInspectorGui(World* world) {
    bool changed = false;
    if (ImGui::Checkbox("Is Active", &isActive)) changed = true;
    const char* typeStr = "Unknown";
    switch (GetType()) {
        case CIRCLE: typeStr = "Circle"; break;
        case BOX: typeStr = "Box"; break;
        case POLYGON: typeStr = "Polygon"; break;
    }
    ImGui::Text("Type: %s", typeStr);

    if (world && owner) {
        ImGui::Separator();
        ImGui::TextDisabled("Ignored Collisions");
        
        auto& ignored = owner->GetIgnoredIDs();
        std::vector<size_t> toRemove;
        
        for (size_t id : ignored) {
            GameObject* ignoredObj = nullptr;
            for (const auto& objPtr : world->GetGameObjects()) {
                if (objPtr->GetID() == id) {
                    ignoredObj = objPtr.get();
                    break;
                }
            }
            
            std::string name = ignoredObj ? ignoredObj->GetName() : ("Unknown ID: " + std::to_string(id));
            ImGui::BulletText("%s", name.c_str());
            ImGui::SameLine();
            ImGui::PushID((int)id);
            if (ImGui::Button("X")) {
                toRemove.push_back(id);
                changed = true;
            }
            ImGui::PopID();
        }

        for (size_t id : toRemove) {
            owner->RemoveIgnored(id);
            for (const auto& objPtr : world->GetGameObjects()) {
                if (objPtr->GetID() == id) {
                    objPtr->RemoveIgnored(owner->GetID());
                    break;
                }
            }
        }

        if (ImGui::BeginCombo("##AddIgnore", "Add Object...")) {
            for (const auto& objPtr : world->GetGameObjects()) {
                if (objPtr.get() == owner) continue;
                // Ignore "INTERNAL_WALL" or generators if desired, but let's allow anything for now, maybe exclude INTERNAL_WALL
                if (objPtr->GetGroupName() == "INTERNAL_WALL") continue;
                if (ignored.count(objPtr->GetID())) continue;

                if (ImGui::Selectable(objPtr->GetName().c_str())) {
                    owner->AddIgnored(objPtr->GetID());
                    objPtr->AddIgnored(owner->GetID());
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
    }

    return changed;
}

void Collider::Toggle() {
    isActive = !isActive;
}

