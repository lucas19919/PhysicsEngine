#include "main/components/collidertypes/PolygonCollider.h"
#include "external/imgui/imgui.h"
#include "math/RotationMatrix.h"
#include <algorithm>

PolygonCollider::PolygonCollider(const Array<20>& vertices) : vertices(vertices) {}

ColliderType PolygonCollider::GetType() const { return POLYGON; }

void PolygonCollider::UpdateCache(const TransformComponent& transform) {
    cachedVertices.Clear();
    RotMatrix rot(transform.rotation);
    Vec2 pos = transform.position;

    for (size_t i = 0; i < vertices.Size(); i++) {
        cachedVertices.PushBack(pos + rot.Rotate(vertices[i]));
    }

    cachedNormals.Clear();
    for (size_t i = 0; i < cachedVertices.Size(); i++) {
        Vec2 edge = cachedVertices[(i + 1) % cachedVertices.Size()] - cachedVertices[i];
        cachedNormals.PushBack(Vec2(edge.y, -edge.x).Norm());
    }

    if (cachedVertices.Size() > 0) {
        bounds.min = cachedVertices[0];
        bounds.max = cachedVertices[0];
        for (size_t i = 1; i < cachedVertices.Size(); i++) {
            bounds.min.x = std::min(bounds.min.x, cachedVertices[i].x);
            bounds.min.y = std::min(bounds.min.y, cachedVertices[i].y);
            bounds.max.x = std::max(bounds.max.x, cachedVertices[i].x);
            bounds.max.y = std::max(bounds.max.y, cachedVertices[i].y);
        }
    }
}

bool PolygonCollider::TestPoint(Vec2 point) const {
    for (size_t i = 0; i < cachedVertices.Size(); i++) {
        Vec2 v = cachedVertices[i];
        Vec2 n = cachedNormals[i];
        if (n.Dot(point - v) > 0) return false;
    }
    return true;
}

void PolygonCollider::Scale(float sx, float sy) {
    for (size_t i = 0; i < vertices.Size(); i++) {
        vertices[i].x *= sx;
        vertices[i].y *= sy;
    }
}

bool PolygonCollider::OnInspectorGui(World* world) {
    bool changed = Collider::OnInspectorGui(world);
    ImGui::Text("Polygon Vertices: %zu", vertices.Size());
    return changed;
}
