#include "main/components/collidertypes/BoxCollider.h"
#include "external/imgui/imgui.h"
#include "math/RotationMatrix.h"
#include <algorithm>

BoxCollider::BoxCollider(Vec2 size) : size(size) {}

ColliderType BoxCollider::GetType() const { return BOX; }

void BoxCollider::UpdateCache(const TransformComponent& transform) {
    cachedVertices.Clear();
    float w = size.x / 2.0f;
    float h = size.y / 2.0f;
    
    // Corners in local space
    Vec2 corners[4] = {
        {-w, -h}, {w, -h}, {w, h}, {-w, h}
    };

    RotMatrix rot(transform.rotation);
    Vec2 pos = transform.position;

    for (int i = 0; i < 4; i++) {
        cachedVertices.PushBack(pos + rot.Rotate(corners[i]));
    }

    cachedNormals.Clear();
    for (int i = 0; i < 4; i++) {
        Vec2 edge = cachedVertices[(i + 1) % 4] - cachedVertices[i];
        cachedNormals.PushBack(Vec2(edge.y, -edge.x).Norm());
    }

    bounds.min = cachedVertices[0];
    bounds.max = cachedVertices[0];
    for (int i = 1; i < 4; i++) {
        bounds.min.x = std::min(bounds.min.x, cachedVertices[i].x);
        bounds.min.y = std::min(bounds.min.y, cachedVertices[i].y);
        bounds.max.x = std::max(bounds.max.x, cachedVertices[i].x);
        bounds.max.y = std::max(bounds.max.y, cachedVertices[i].y);
    }
}

bool BoxCollider::TestPoint(Vec2 point) const {
    // Basic point-in-polygon test (since it's an AABB/OBB)
    for (size_t i = 0; i < cachedVertices.Size(); i++) {
        Vec2 v = cachedVertices[i];
        Vec2 n = cachedNormals[i];
        if (n.Dot(point - v) > 0) return false;
    }
    return true;
}

void BoxCollider::Scale(float sx, float sy) {
    size.x *= sx;
    size.y *= sy;
}

bool BoxCollider::OnInspectorGui(World* world) {
    bool changed = Collider::OnInspectorGui(world);
    if (ImGui::DragFloat2("Size", &size.x, 0.1f, 0.01f, 100.0f)) changed = true;
    return changed;
}
