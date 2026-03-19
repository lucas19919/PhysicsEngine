#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/Collider.h"

BoxCollider::BoxCollider(Vec2 size) : size(size) {}

ColliderType BoxCollider::getType() const
{
    return ColliderType::BOX;
}