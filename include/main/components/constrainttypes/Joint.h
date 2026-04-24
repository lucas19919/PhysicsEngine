#pragma once
#include "math/Vec2.h"
#include "main/components/Constraint.h"
#include <vector>

class GameObject;

struct JointAttachment
{
    GameObject* obj;
    Vec2 localAnchor;
};

class JointConstraint : public Constraint
{
    public:
        std::vector<JointAttachment> attachments;
        bool collisions;

        JointConstraint(std::vector<JointAttachment> attachments, Vec2 position, bool collisions);
        ConstraintType GetType() const override;
        const char* GetName() const override { return "JointConstraint"; }

        void Solve(float dt) override;

    private:
        void SingleJoint(float dt);
        void ComplexJoint(float dt);
};