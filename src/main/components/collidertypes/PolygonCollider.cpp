#include "main/components/collidertypes/PolygonCollider.h"
#include <cmath>

PolygonCollider::PolygonCollider(const Array<20>& vertices) : vertices(vertices) {}

ColliderType PolygonCollider::GetType() const
{
    return ColliderType::POLYGON;
}

#include "math/RotationMatrix.h"

void PolygonCollider::UpdateCache(const TransformComponent& transform)
{
    cachedVertices.count = 0;
    cachedNormals.count = 0;

    RotMatrix rot(transform.rotation);

    for (size_t i = 0; i < vertices.Size(); i++)
    {
        Vec2 rotated = rot.Rotate(vertices[i]);
        cachedVertices.PushBack(rotated + transform.position);
    }

    // Update normals
    for (size_t i = 0; i < cachedVertices.Size(); i++)
    {
        Vec2 edge = cachedVertices[(i + 1) % cachedVertices.Size()] - cachedVertices[i];
        Vec2 normal = Vec2(edge.y, -edge.x);
        cachedNormals.PushBack(normal.Norm());
    }
}