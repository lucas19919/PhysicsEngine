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

        PinConstraint(const std::vector<PinAttachment>& attachments, Vec2 pos, bool fixedX, bool fixedY);
        ConstraintType GetType() const override;
        void Solve(float dt) override;
};