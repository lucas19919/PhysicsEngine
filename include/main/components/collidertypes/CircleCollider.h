#pragma once
#include "math/Vec2.h"
#include "main/components/Collider.h"

class CircleCollider : public Collider
{
    public:
        float radius;    
        CircleCollider(float r);
        ColliderType GetType() const override;
        
        void UpdateCache(const TransformComponent& transform) override {}
        bool TestPoint(Vec2 point) const override;
};