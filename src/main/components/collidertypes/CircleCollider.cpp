#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/Collider.h"

CircleCollider::CircleCollider(float radius) : radius(radius) {}

ColliderType CircleCollider::GetType() const
{
    return ColliderType::CIRCLE;
}