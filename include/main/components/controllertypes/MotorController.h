#pragma once
#include "main/components/Controller.h"
#include "main/components/constrainttypes/Motor.h"

class GameObject;

class MotorController : public Controller
{
    public:
        MotorController(bool active, std::vector<MotorConstraint*> motors, float torqueMax); 
        ControllerType GetType() const override;
        void Update(float dt) override;

    private:
        std::vector<MotorConstraint*> motors;
        float torqueMax;
};