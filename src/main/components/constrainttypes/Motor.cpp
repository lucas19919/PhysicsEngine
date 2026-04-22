#include "main/components/constrainttypes/Motor.h"
#include "main/components/Constraint.h"
#include "main/World.h"
#include "main/physics/Config.h"
#include "math/RotationMatrix.h"

MotorConstraint::MotorConstraint(GameObject* rotor, Vec2 localPosition, float torque)
    : rotor(rotor), localPosition(localPosition), torque(torque)
{ 
    this->position = rotor->transform.position + localPosition;
}

ConstraintType MotorConstraint::GetType() const
{
    return ConstraintType::MOTOR;
}

//if a body doesnt have a rigidbody, torque = angular velocity
//NOTE: this means that if you dont have a rigidbody, bodies will rotate through other objects 
void MotorConstraint::Solve(float dt)
{
    RigidBody* rb = rotor->rb;

    if (!rb)
    {
        float angleChange = torque * dt;
        rotor->transform.rotation += angleChange;

        RotMatrix rot(rotor->transform.rotation);
        rotor->transform.position = this->position - rot.Rotate(localPosition);
    }
    else
    {
        float iterationImpulse = torque * dt;
        
        float newAngularVel = rb->GetAngularVelocity() + (iterationImpulse * rb->GetInvInertia());
        rb->SetAngularVelocity(newAngularVel);
    }
}