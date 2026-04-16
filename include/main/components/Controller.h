#pragma once
#include "main/components/Constraint.h"
#include "main/components/Component.h"
#include <vector>

enum ControllerType
{
    CNT_MOTOR
};

struct Controller : public Component
{
    bool active;

    virtual ~Controller() = default;
    virtual ControllerType GetType() const = 0;
    virtual void Update(float dt) = 0;
};