#pragma once
#include "main/components/Collider.h"
#include "math/Vec2.h"

class BoxCollider : public Collider
{
    public:
        Vec2 size;
        BoxCollider(Vec2 size);
        ColliderType GetType() const override;
        const char* GetName() const override { return "BoxCollider"; }
void UpdateCache(const TransformComponent& transform) override;
bool TestPoint(Vec2 point) const override;
void Scale(float sx, float sy) override;

bool OnInspectorGui(class World* world = nullptr) override;
};