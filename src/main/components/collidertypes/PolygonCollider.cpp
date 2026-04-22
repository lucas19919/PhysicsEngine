#include "main/components/collidertypes/PolygonCollider.h"
#include <cmath>

PolygonCollider::PolygonCollider(const Array<20>& vertices) : vertices(vertices) {}

ColliderType PolygonCollider::GetType() const
{
    return ColliderType::POLYGON;
}

void PolygonCollider::UpdateCache(const TransformComponent& transform)
{
    cachedVertices.count = 0;
    cachedNormals.count = 0;

    float cos = std::cos(transform.rotation);
    float sin = std::sin(transform.rotation);

    for (size_t i = 0; i < vertices.Size(); i++)
    {
        float x = vertices[i].x;
        float y = vertices[i].y;

        cachedVertices.PushBack(Vec2(
            (x * cos) - (y * sin) + transform.position.x,
            (x * sin) + (y * cos) + transform.position.y
        ));
    }

    // Update normals
    for (size_t i = 0; i < cachedVertices.Size(); i++)
    {
        Vec2 edge = cachedVertices[(i + 1) % cachedVertices.Size()] - cachedVertices[i];
        Vec2 normal = Vec2(edge.y, -edge.x);
        cachedNormals.PushBack(normal.Norm());
    }
}