#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/Collider.h"
#include "main/GameObject.h"

CircleCollider::CircleCollider(float radius) : radius(radius) {}

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