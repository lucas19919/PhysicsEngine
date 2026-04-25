#pragma once
#include "math/Vec2.h"
#include "main/components/Constraint.h"
#include <vector>

class GameObject;

struct PinAttachment
{
    GameObject* obj;
    Vec2 localAnchor;
};

class PinConstraint : public Constraint
{
    public:
        std::vector<PinAttachment> attachments;

        bool fixedX = true;
        bool fixedY = true;

        PinConstraint(std::vector<PinAttachment> attachments, Vec2 position, bool fixedX, bool fixedY);
        ConstraintType GetType() const override;
        const char* GetName() const override { return "PinConstraint"; }

        void Solve(float dt) override;
        void OnObjectRemoved(size_t id) override;
        bool IsInvalid() const override;
};