#include "main/components/collidertypes/PolygonCollider.h"
#include "main/components/Collider.h"
#include "main/utility/templates/Array.h"

PolygonCollider::PolygonCollider(const Array<20>& vertices) : vertices(vertices) {}

ColliderType PolygonCollider::GetType() const
{
    return ColliderType::POLYGON;
}