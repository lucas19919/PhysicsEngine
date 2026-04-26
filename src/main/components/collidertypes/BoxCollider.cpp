#include "main/components/collidertypes/BoxCollider.h"
#include "main/GameObject.h"
#include <cmath>
#include "external/imgui/imgui.h"

BoxCollider::BoxCollider(Vec2 size) : size(size) {}

bool BoxCollider::OnInspectorGui(World* world)
{
    bool changed = Collider::OnInspectorGui(world);
    if (ImGui::DragFloat2("Size", &size.x, 0.1f, 0.01f, 100.0f))
    {
        if (owner) UpdateCache(owner->transform);
        changed = true;
    }
    return changed;
}

ColliderType BoxCollider::GetType() const
{
    return ColliderType::BOX;
}

#include "math/RotationMatrix.h"

void BoxCollider::UpdateCache(const TransformComponent& transform)
{
    cachedVertices.count = 0;
    cachedNormals.count = 0;

    float x = size.x / 2.0f;
    float y = size.y / 2.0f;
    
    Vec2 local[4] = {
        Vec2(-x, -y), Vec2( x, -y), Vec2( x,  y), Vec2(-x,  y)
    };

    RotMatrix rot(transform.rotation);

    for (int i = 0; i < 4; i++)
    {
        Vec2 rotated = rot.Rotate(local[i]);
        cachedVertices.PushBack(rotated + transform.position);
    }

    // Update normals
    for (size_t i = 0; i < 4; i++)
    {
        Vec2 edge = cachedVertices[(i + 1) % 4] - cachedVertices[i];
        Vec2 normal = Vec2(edge.y, -edge.x);
        cachedNormals.PushBack(normal.Norm());
    }
}

bool BoxCollider::TestPoint(Vec2 point) const
{
    for (size_t i = 0; i < cachedVertices.Size(); i++)
    {
        Vec2 v = cachedVertices[i];
        Vec2 n = cachedNormals[i];
        if (n.Dot(point - v) > 0.0f) return false;
    }
    return true;
}