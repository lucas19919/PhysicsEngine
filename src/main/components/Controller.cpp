#include "main/components/Controller.h"

#include "external/imgui/imgui.h"

bool Controller::OnInspectorGui(World* world) {
    bool changed = false;
    if (ImGui::Checkbox("Active", &active)) changed = true;
    return changed;
}
