#include "main/components/collidertypes/CircleCollider.h"
#include "main/GameObject.h"
#include "external/imgui/imgui.h"

CircleCollider::CircleCollider(float radius) : radius(radius) {}

ColliderType CircleCollider::GetType() const { return CIRCLE; }

void CircleCollider::UpdateCache(const TransformComponent& transform) {
    cachedVertices.Clear();
    cachedNormals.Clear();
    
    bounds.min = transform.position - Vec2(radius, radius);
    bounds.max = transform.position + Vec2(radius, radius);
}

bool CircleCollider::TestPoint(Vec2 point) const {
    if (!owner) return false;
    return (point - owner->transform.position).MagSq() <= (radius * radius);
}

void CircleCollider::Scale(float sx, float sy) {
    radius *= (sx + sy) / 2.0f;
}

bool CircleCollider::OnInspectorGui(World* world) {
    bool changed = Collider::OnInspectorGui(world);
    if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.01f, 100.0f)) changed = true;
    return changed;
}
