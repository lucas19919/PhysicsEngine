#include "main/components/constrainttypes/Distance.h"
#include "main/components/Constraint.h"
#include "main/World.h"
#include "main/physics/Config.h"
#include "math/RotationMatrix.h"


DistanceConstraint::DistanceConstraint(GameObject* anchor, GameObject* attached, float length, Vec2 anchorOffset, Vec2 attachedOffset) 
: anchor(anchor), attached(attached), length(length), anchorOffset(anchorOffset), attachedOffset(attachedOffset)
{ }

ConstraintType DistanceConstraint::GetType() const
{
    return ConstraintType::DISTANCE;
}

void DistanceConstraint::Solve(float dt)
{
    RigidBody* rb1 = anchor->rb;
    RigidBody* rb2 = attached->rb;

    float invMass1 = (rb1) ? rb1->GetInvMass() : 0.0f;
    float invMass2 = (rb2) ? rb2->GetInvMass() : 0.0f;
    float invInertia1 = (rb1) ? rb1->GetInvInertia() : 0.0f;
    float invInertia2 = (rb2) ? rb2->GetInvInertia() : 0.0f;

    if (invMass1 == 0.0f && invMass2 == 0.0f && invInertia1 == 0.0f && invInertia2 == 0.0f) return;

    RotMatrix rot1(anchor->transform.rotation);
    Vec2 r1 = rot1.Rotate(anchorOffset);
    RotMatrix rot2(attached->transform.rotation);
    Vec2 r2 = rot2.Rotate(attachedOffset);

    Vec2 p1 = anchor->transform.position + r1;
    Vec2 p2 = attached->transform.position + r2;

    Vec2 distVector = p2 - p1;
    float distance = distVector.Mag();

    if (distance < 0.0001f) return; //avoid division by zero

    Vec2 axis = distVector * (1.0f / distance);
    float error = distance - length;

    Vec2 v1 = rb1 ? rb1->GetVelocity() : Vec2(0,0);
    float w1 = rb1 ? rb1->GetAngularVelocity() : 0.0f;
    Vec2 v2 = rb2 ? rb2->GetVelocity() : Vec2(0,0);
    float w2 = rb2 ? rb2->GetAngularVelocity() : 0.0f;

    Vec2 vp1 = v1 + Vec2(-w1 * r1.y, w1 * r1.x);
    Vec2 vp2 = v2 + Vec2(-w2 * r2.y, w2 * r2.x);

    Vec2 relative = vp2 - vp1;
    float magnitude = axis.Dot(relative);

    float bias = (Config::biasConstraint / dt) * error;

    float r1c = r1.Cross(axis);
    float r2c = r2.Cross(axis);
    float denominator = invMass1 + invMass2 + (r1c * r1c * invInertia1) + (r2c * r2c * invInertia2);

    if (denominator <= 0.0f) return;

    float j = -(magnitude + bias) / denominator;
    Vec2 impulse = axis * j;

    if (rb1) 
    {
        rb1->SetVelocity(v1 - impulse * invMass1);
        rb1->SetAngularVelocity(w1 - (r1.Cross(impulse) * invInertia1));
    }
    if (rb2) 
    {
        rb2->SetVelocity(v2 + impulse * invMass2);
        rb2->SetAngularVelocity(w2 + (r2.Cross(impulse) * invInertia2));
    }
}