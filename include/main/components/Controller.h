#pragma once
#include <vector>

#include "main/components/Component.h"
#include "main/components/Constraint.h"

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

    const char* GetName() const override { return "Controller"; }
    bool OnInspectorGui(class World* world = nullptr) override;
};