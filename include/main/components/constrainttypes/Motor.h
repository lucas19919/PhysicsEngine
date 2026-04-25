#pragma once
#include "math/Vec2.h"
#include "main/components/Constraint.h"
#include "main/components/constrainttypes/Pin.h"
#include <vector>

class GameObject;

class MotorConstraint : public Constraint
{
    public:
        GameObject* rotor;

        float torque;
        Vec2 localPosition; //relative to rotor center

        MotorConstraint(GameObject* rotor, Vec2 localPosition, float torque);
        ConstraintType GetType() const override;
        const char* GetName() const override { return "MotorConstraint"; }

        void Solve(float dt) override;
        void OnObjectRemoved(size_t id) override;
};