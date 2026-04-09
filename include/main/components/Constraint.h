#pragma once
#include "math/Vec2.h"

class GameObject;

enum ConstraintType
{
    DISTANCE,
    PIN,
    JOINT,
    MOTOR
};

class Constraint
{
    public: 
        Vec2 position;

        virtual ~Constraint() = default;
        virtual ConstraintType GetType() const = 0;
        virtual void Solve(float dt) = 0;
};