#include "main/components/controllertypes/MotorController.h"
#include "main/components/constrainttypes/Motor.h"
#include "main/components/Constraint.h"
#include "raylib.h"

MotorController::MotorController(bool active, std::vector<MotorConstraint*> motors, float torqueMax)
{
    this->active = active;
    this->motors = motors;
    this->torqueMax = torqueMax;
}

ControllerType MotorController::GetType() const
{
    return ControllerType::CNT_MOTOR;
}

void MotorController::Update(float dt)
{
    for (MotorConstraint* motor : motors)
    {
        if (IsKeyDown(KEY_D))
        {
            motor->torque = torqueMax;
        }
        else if (IsKeyDown(KEY_A))
        {
            motor->torque = -torqueMax;
        }
        else
        {
            motor->torque = 0.0f;
        }
    }
}