#include "main/physics/Solver.h"
#include "main/GameObject.h"
#include "main/physics/SAT.h"
#include "main/physics/ManifoldHandler.h"
#include "main/physics/Config.h"
#include <algorithm>

CollisionManifold Solver::ResolveManifold(GameObject* obj1, GameObject* obj2)
{
    return ManifoldHandler::SortManifold(obj1, obj2);
}

void Solver::ResolveConstraints(ContactConstraint& contact)
{
    if (contact.pointCount == 0) return;

    RigidBody* rb1 = contact.rb1;
    RigidBody* rb2 = contact.rb2;

    float invMass1 = rb1 ? rb1->GetInvMass() : 0.0f;
    float invMass2 = rb2 ? rb2->GetInvMass() : 0.0f;
    float invInertia1 = rb1 ? rb1->GetInvInertia() : 0.0f;
    float invInertia2 = rb2 ? rb2->GetInvInertia() : 0.0f;

    Vec2 normal = contact.normal;
    Vec2 tangent = Vec2(-normal.y, normal.x);

    for (int i = 0; i < contact.pointCount; i++)
    {
        Vec2 r1 = contact.r1[i];
        Vec2 r2 = contact.r2[i];

        //relative velocity
        Vec2 v1 = rb1 ? rb1->GetVelocity() : Vec2(0,0);
        Vec2 v2 = rb2 ? rb2->GetVelocity() : Vec2(0,0);
        float w1 = rb1 ? rb1->GetAngularVelocity() : 0.0f;
        float w2 = rb2 ? rb2->GetAngularVelocity() : 0.0f;

        Vec2 dv = (v2 + Vec2(-w2 * r2.y, w2 * r2.x)) - (v1 + Vec2(-w1 * r1.y, w1 * r1.x));

        //normal impulse
        float vn = dv.Dot(normal);
        float jn = (contact.restitutionBias[i] - vn) * contact.normalMass[i];

        float oldJN = contact.accumulatedNormalImpulse[i];
        contact.accumulatedNormalImpulse[i] = std::max(oldJN + jn, 0.0f);
        float deltaJN = contact.accumulatedNormalImpulse[i] - oldJN;

        //tangent impulse
        float vt = dv.Dot(tangent);
        float jt = -vt * contact.tangentMass[i];

        float maxFriction = contact.friction * contact.accumulatedNormalImpulse[i];
        float oldJT = contact.accumulatedTangentImpulse[i];
        contact.accumulatedTangentImpulse[i] = std::clamp(oldJT + jt, -maxFriction, maxFriction);
        float deltaJT = contact.accumulatedTangentImpulse[i] - oldJT;

        //apply impulses
        Vec2 impulse = normal * deltaJN + tangent * deltaJT;

        if (rb1)
        {
            rb1->SetVelocity(rb1->GetVelocity() - impulse * invMass1);
            rb1->SetAngularVelocity(rb1->GetAngularVelocity() - r1.Cross(impulse) * invInertia1);
        }
        if (rb2)
        {
            rb2->SetVelocity(rb2->GetVelocity() + impulse * invMass2);
            rb2->SetAngularVelocity(rb2->GetAngularVelocity() + r2.Cross(impulse) * invInertia2);
        }
    }
}

void Solver::ResolvePosition(ContactConstraint& contact)
{
    RigidBody* rb1 = contact.rb1;
    RigidBody* rb2 = contact.rb2;

    float invMass1 = (rb1) ? rb1->GetInvMass() : 0.0f;
    float invMass2 = (rb2) ? rb2->GetInvMass() : 0.0f;

    float totalInvMass = invMass1 + invMass2;
    if (totalInvMass == 0.0f) return;

    float slop = Config::contactSlop;
    float percent = Config::positionCorrectionPercent;

    float penetration = std::max(contact.penetration - slop, 0.0f);
    Vec2 correction = contact.normal * (penetration / totalInvMass) * percent;   

    if (rb1)
    {
        contact.obj1->transform.position = contact.obj1->transform.position - (correction * invMass1);
        contact.obj1->transform.isDirty = true;
    }
    if (rb2)
    {
        contact.obj2->transform.position = contact.obj2->transform.position + (correction * invMass2);
        contact.obj2->transform.isDirty = true;
    }
}

void Solver::Warmstart(ContactConstraint& contact)
{
    RigidBody* rb1 = contact.rb1;
    RigidBody* rb2 = contact.rb2;

    float invMass1 = rb1 ? rb1->GetInvMass() : 0.0f;
    float invMass2 = rb2 ? rb2->GetInvMass() : 0.0f;
    float invInertia1 = rb1 ? rb1->GetInvInertia() : 0.0f;
    float invInertia2 = rb2 ? rb2->GetInvInertia() : 0.0f;

    Vec2 normal = contact.normal;
    Vec2 tangent = Vec2(-normal.y, normal.x);

    for (int i = 0; i < contact.pointCount; i++)
    {
        Vec2 impulse = normal * contact.accumulatedNormalImpulse[i] + tangent * contact.accumulatedTangentImpulse[i];
        Vec2 r1 = contact.r1[i];
        Vec2 r2 = contact.r2[i];

        if (rb1)
        {
            rb1->SetVelocity(rb1->GetVelocity() - impulse * invMass1);
            rb1->SetAngularVelocity(rb1->GetAngularVelocity() - r1.Cross(impulse) * invInertia1);
        }
        if (rb2)
        {
            rb2->SetVelocity(rb2->GetVelocity() + impulse * invMass2);
            rb2->SetAngularVelocity(rb2->GetAngularVelocity() + r2.Cross(impulse) * invInertia2);
        }
    }
}
