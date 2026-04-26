#pragma once
#include "math/Vec2.h"
#include "main/components/Constraint.h"

class GameObject;

class DistanceConstraint : public Constraint
{
    public:
        GameObject* anchor;
        GameObject* attached;

        Vec2 anchorOffset;
        Vec2 attachedOffset;

        float length;

        DistanceConstraint(GameObject* anchor, GameObject* attached, float length, Vec2 anchorOffset, Vec2 attachedOffset);
        ConstraintType GetType() const override;
        const char* GetName() const override { return "DistanceConstraint"; }

        void Solve(float dt) override;
        void OnObjectRemoved(size_t id) override;
        bool InvolvesObject(GameObject* obj) const override;
        bool OnInspectorGui(class World* world = nullptr) override;
};