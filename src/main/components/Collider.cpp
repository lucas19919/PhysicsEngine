#include "main/components/Collider.h"

#include "external/imgui/imgui.h"

void Collider::OnInspectorGui() {
    ImGui::Checkbox("Is Active", &isActive);
    const char* typeStr = "Unknown";
    switch (GetType()) {
        case CIRCLE: typeStr = "Circle"; break;
        case BOX: typeStr = "Box"; break;
        case POLYGON: typeStr = "Polygon"; break;
    }
    ImGui::Text("Type: %s", typeStr);
}

void Collider::Toggle() {
    isActive = !isActive;
}
