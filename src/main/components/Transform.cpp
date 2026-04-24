#include "main/components/TransformComponent.h"
#include "external/imgui/imgui.h"

void TransformComponent::OnInspectorGui() {
    if (ImGui::DragFloat2("Position", &position.x, 0.1f)) {
        isDirty = true;
    }
    float rotDeg = rotation * 57.29578f; // RAD2DEG
    if (ImGui::DragFloat("Rotation", &rotDeg, 1.0f)) {
        rotation = rotDeg * 0.0174533f; // DEG2RAD
        isDirty = true;
    }
}

TransformComponent::TransformComponent()
{
    position = Vec2();
    rotation = 0.0f;
}