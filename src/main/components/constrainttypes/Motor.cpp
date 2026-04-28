#include "main/components/constrainttypes/Motor.h"

#include "external/imgui/imgui.h"

#include "main/GameObject.h"
#include "main/World.h"
#include "main/components/Constraint.h"
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

void MotorConstraint::OnObjectRemoved(size_t id)
{
    Component::OnObjectRemoved(id);
    if (rotor->GetID() == id)
    {
        isComponentDeleted = true;
    }
}

bool MotorConstraint::InvolvesObject(GameObject* obj) const
{
    return rotor == obj;
}


bool MotorConstraint::OnInspectorGui(World* world)
{
    ImGui::Text("Type: Motor");
    ImGui::Text("Rotor: %s (ID: %zu)", rotor->GetName().c_str(), rotor->GetID());

    bool changed = false;
    if (ImGui::DragFloat("Torque", &torque, 1.0f)) changed = true;
    if (ImGui::DragFloat2("Local Pos", &localPosition.x, 0.05f)) changed = true;

    return changed;
}