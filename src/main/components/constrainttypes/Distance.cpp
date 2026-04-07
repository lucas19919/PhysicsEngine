#include "main/components/constrainttypes/Distance.h"
#include "main/components/Constraint.h"
#include "main/World.h"
#include "main/physics/Config.h"


DistanceConstraint::DistanceConstraint(GameObject* anchor, GameObject* attached, float length) : anchor(anchor), attached(attached), length(length)
{ }

ConstraintType DistanceConstraint::GetType() const
{
    return ConstraintType::DISTANCE;
}

void DistanceConstraint::Solve(float dt)
{
    RigidBody* rb1 = anchor->GetRigidBody();
    RigidBody* rb2 = attached->GetRigidBody();

    float invMass1 = (rb1) ? rb1->GetInvMass() : 0.0f;
    float invMass2 = (rb2) ? rb2->GetInvMass() : 0.0f;

    if (invMass1 == 0.0f && invMass2 == 0.0f) return;

    Vec2 distVector = attached->transform.position - anchor->transform.position;
    float distance = distVector.Mag();
    if (distance < length) return; 

    Vec2 axis = distVector * (1.0f / distance);
    float error = distance - length;

    Vec2 v1 = rb1 ? rb1->GetVelocity() : Vec2(0,0);
    Vec2 v2 = rb2 ? rb2->GetVelocity() : Vec2(0,0);

    Vec2 relative = v2 - v1;
    float magnitude = axis.Dot(relative);

    float bias = (Config().biasConstraint / dt) * error;
    float totalInvMass = invMass1 + invMass2;

    if (totalInvMass == 0.0f) return;

    float j = -(magnitude + bias) / totalInvMass;

    Vec2 impulse = axis * j;

    if (rb1) rb1->SetVelocity(rb1->GetVelocity() - impulse * invMass1);
    if (rb2) rb2->SetVelocity(rb2->GetVelocity() + impulse * invMass2);
}