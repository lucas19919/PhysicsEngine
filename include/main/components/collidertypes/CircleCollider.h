#pragma once
#include "math/Vec2.h"
#include "main/components/Collider.h"

class CircleCollider : public Collider
{
    public:
        float radius;    
        CircleCollider(float radius);
        ColliderType GetType() const override;
        const char* GetName() const override { return "CircleCollider"; }

        void UpdateCache(const TransformComponent& transform) override;
        bool TestPoint(Vec2 point) const override;
};