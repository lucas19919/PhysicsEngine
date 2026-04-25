#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/Collider.h"
#include "main/GameObject.h"
#include "external/imgui/imgui.h"

CircleCollider::CircleCollider(float radius) : radius(radius) {}

void CircleCollider::OnInspectorGui()
{
    Collider::OnInspectorGui();
    if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.01f, 100.0f))
    {
        if (owner) UpdateCache(owner->transform);
    }
}

ColliderType CircleCollider::GetType() const
{
    return ColliderType::CIRCLE;
}

void CircleCollider::UpdateCache(const TransformComponent& transform)
{
    bounds.min = transform.position - Vec2(radius, radius);
    bounds.max = transform.position + Vec2(radius, radius);
}

bool CircleCollider::TestPoint(Vec2 point) const
{
    if (!owner) return false;
    return (point - owner->transform.position).MagSq() <= radius * radius;
}