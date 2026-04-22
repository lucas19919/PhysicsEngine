#include "main/components/collidertypes/BoxCollider.h"
#include <cmath>

BoxCollider::BoxCollider(Vec2 size) : size(size) {}

ColliderType BoxCollider::GetType() const
{
    return ColliderType::BOX;
}

void BoxCollider::UpdateCache(const TransformComponent& transform)
{
    cachedVertices.count = 0;
    cachedNormals.count = 0;

    float x = size.x / 2.0f;
    float y = size.y / 2.0f;
    
    Vec2 local[4] = {
        Vec2(-x, -y), Vec2( x, -y), Vec2( x,  y), Vec2(-x,  y)
    };

    float cos = std::cos(transform.rotation);
    float sin = std::sin(transform.rotation);

    for (int i = 0; i < 4; i++)
    {
        cachedVertices.PushBack(Vec2(
            (local[i].x * cos) - (local[i].y * sin) + transform.position.x,
            (local[i].x * sin) + (local[i].y * cos) + transform.position.y
        ));
    }

    // Update normals
    for (size_t i = 0; i < 4; i++)
    {
        Vec2 edge = cachedVertices[(i + 1) % 4] - cachedVertices[i];
        Vec2 normal = Vec2(edge.y, -edge.x);
        cachedNormals.PushBack(normal.Norm());
    }
}