#pragma once
#include "math/Vec2.h"
#include "main/components/Constraint.h"
#include <vector>

class GameObject;

struct JointAttachment
{
    GameObject* obj;
    float localX;
    float localY;
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
        void OnObjectRemoved(size_t id) override;
        bool IsInvalid() const override;
        bool InvolvesObject(GameObject* obj) const override;
        bool OnInspectorGui(class World* world = nullptr) override;

    private:
        void SingleJoint(float dt);
        void ComplexJoint(float dt);
};