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
        const char* GetName() const override { return "MotorController"; }

        const std::vector<MotorConstraint*>& GetMotors() const { return motors; }
        void RemoveMotor(MotorConstraint* motor);
        float GetTorqueMax() const { return torqueMax; }

        void OnObjectRemoved(size_t id) override;
        bool IsInvalid() const override;

    private:
        std::vector<MotorConstraint*> motors;
        float torqueMax;
};