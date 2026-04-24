#include "main/components/Controller.h"
#include "external/imgui/imgui.h"

void Controller::OnInspectorGui() {
    ImGui::Checkbox("Active", &active);
}
