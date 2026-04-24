#pragma once
#include "math/Vec2.h"
#include "main/components/Collider.h"

class BoxCollider : public Collider
{
    public:
        Vec2 size;
        BoxCollider(Vec2 size);
        ColliderType GetType() const override;
        const char* GetName() const override { return "BoxCollider"; }

        void UpdateCache(const TransformComponent& transform) override;
        bool TestPoint(Vec2 point) const override;
};