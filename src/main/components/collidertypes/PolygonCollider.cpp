#include "main/components/collidertypes/PolygonCollider.h"
#include "main/components/Collider.h"

PolygonCollider::PolygonCollider(const std::vector<Vec2>& vertices) : vertices(vertices) {}

ColliderType PolygonCollider::GetType() const
{
    return ColliderType::POLYGON;
}