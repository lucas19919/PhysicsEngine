#include "main/components/controllertypes/MotorController.h"

#include <algorithm>

#include "raylib.h"

#include "main/GameObject.h"
#include "main/World.h"
#include "main/components/Constraint.h"
#include "main/components/constrainttypes/Motor.h"

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

void MotorController::RemoveMotor(MotorConstraint* motor)
{
    motors.erase(std::remove(motors.begin(), motors.end(), motor), motors.end());
}

void MotorController::OnObjectRemoved(size_t id)
{
    // 1. Handle base component logic (owner deletion)
    Component::OnObjectRemoved(id);
    
    // 2. Filter out motors whose rotor is being removed
    motors.erase(std::remove_if(motors.begin(), motors.end(), [id](MotorConstraint* m) {
        return m->rotor->GetID() == id;
    }), motors.end());
}

bool MotorController::IsInvalid() const
{
    return isComponentDeleted || motors.empty();
}