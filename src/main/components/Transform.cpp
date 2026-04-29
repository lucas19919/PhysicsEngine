#include "external/imgui/imgui.h"

#include "main/components/TransformComponent.h"

bool TransformComponent::OnInspectorGui(World* world) {
    bool changed = false;
    if (ImGui::DragFloat2("Position", &position.x, 0.1f)) {
        isDirty = true;
        changed = true;
    }
    float rotDeg = rotation * 57.29578f; // RAD2DEG
    if (ImGui::DragFloat("Rotation", &rotDeg, 1.0f)) {
        rotation = rotDeg * 0.0174533f; // DEG2RAD
        isDirty = true;
        changed = true;
    }
    return changed;
}

TransformComponent::TransformComponent()
{
    position = Vec2();
    rotation = 0.0f;
}