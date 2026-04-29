#pragma once
#include "main/components/Collider.h"
#include "main/utility/templates/Array.h"
#include "math/Vec2.h"

class PolygonCollider : public Collider
{
    public:
        Array<20> vertices;
        PolygonCollider(const Array<20>& vertices);
        ColliderType GetType() const override;
        const char* GetName() const override { return "PolygonCollider"; }

        void UpdateCache(const TransformComponent& transform) override;
        bool TestPoint(Vec2 point) const override;
        void Scale(float sx, float sy) override;

        bool OnInspectorGui(class World* world = nullptr) override;
};