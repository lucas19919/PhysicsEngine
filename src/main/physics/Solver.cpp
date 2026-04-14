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

    for (int i = 0; i < contact.pointCount; i++)
    {
        Vec2 impulse = GetImpulse(contact, i);
        float jn = impulse.x;
        float jt = impulse.y;

        float oldJN = contact.accumulatedNormalImpulse[i];
        float newJN = std::max(oldJN + jn, 0.0f);
        float deltaJN = newJN - oldJN;
        contact.accumulatedNormalImpulse[i] = newJN;

        ApplyImpulse(contact, i, deltaJN, 0.0f);

        float maxFriction = contact.friction * contact.accumulatedNormalImpulse[i];
        float oldJT = contact.accumulatedTangentImpulse[i];
        float newJT = std::clamp(oldJT + jt, -maxFriction, maxFriction);
        float deltaJT = newJT - oldJT;
        contact.accumulatedTangentImpulse[i] = newJT;

        ApplyImpulse(contact, i, 0.0f, deltaJT);
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
    }
    if (rb2)
    {
        contact.obj2->transform.position = contact.obj2->transform.position + (correction * invMass2);
    }
}

void Solver::Warmstart(ContactConstraint& contact)
{
    for (int i = 0; i < contact.pointCount; i++)
    {
        ApplyImpulse(contact, i, contact.accumulatedNormalImpulse[i], contact.accumulatedTangentImpulse[i]);
    }
}

//helpers
Vec2 Solver::GetImpulse(ContactConstraint& contact, int index)
{
    RigidBody *rb1 = contact.rb1;
    RigidBody *rb2 = contact.rb2;

    float invMass1 = (rb1) ? rb1->GetInvMass() : 0.0f;
    float invMass2 = (rb2) ? rb2->GetInvMass() : 0.0f;
    float invInertia1 = rb1 ? rb1->GetInvInertia() : 0.0f;
    float invInertia2 = rb2 ? rb2->GetInvInertia() : 0.0f;

    if (invMass1 == 0.0f && invMass2 == 0.0f) return Vec2();

    Vec2 contactPoint = contact.points[index];

    Vec2 r1 = contactPoint - contact.obj1->transform.position;
    Vec2 r2 = contactPoint - contact.obj2->transform.position;

    Vec2 normal = contact.normal;
    Vec2 tangent = Vec2(-normal.y, normal.x);

    //jn
    Vec2 v1 = rb1 ? rb1->GetVelocity() : Vec2(0,0);
    Vec2 v2 = rb2 ? rb2->GetVelocity() : Vec2(0,0);
    float w1 = rb1 ? rb1->GetAngularVelocity() : 0;
    float w2 = rb2 ? rb2->GetAngularVelocity() : 0;

    Vec2 totalV1 = v1 + Vec2(-w1 * r1.y, w1 * r1.x);
    Vec2 totalV2 = v2 + Vec2(-w2 * r2.y, w2 * r2.x);

    float cross1 = r1.Cross(normal);
    float cross2 = r2.Cross(normal);

    Vec2 relative = totalV2 - totalV1;
    float magnitude = normal.Dot(relative);

    float bias = contact.restitutionBias[index];
    float denominator = invMass1 + invMass2 + (cross1 * cross1 * invInertia1) + (cross2 * cross2 * invInertia2);
    
    float jn = 0.0f;
    if (denominator > 0.0f)
    {
        jn = (bias - magnitude) / denominator;
        jn /= (float)contact.pointCount;
    }

    if (tangent.MagSq() > 0.0001f)
        tangent = tangent.Norm();
    else return Vec2();

    float crossT1 = r1.Cross(tangent);
    float crossT2 = r2.Cross(tangent);

    float jt = -relative.Dot(tangent) / (invMass1 + invMass2 + (crossT1 * crossT1 * invInertia1) + (crossT2 * crossT2 * invInertia2));
    jt /= (float)contact.pointCount;

    return Vec2(jn, jt);
}

void Solver::ApplyImpulse(ContactConstraint& contact, int index, float jn, float jt)
{
    RigidBody* rb1 = contact.rb1;
    RigidBody* rb2 = contact.rb2;

    float invMass1 = rb1 ? rb1->GetInvMass() : 0.0f;
    float invMass2 = rb2 ? rb2->GetInvMass() : 0.0f;
    float invInertia1 = rb1 ? rb1->GetInvInertia() : 0.0f;
    float invInertia2 = rb2 ? rb2->GetInvInertia() : 0.0f;

    Vec2 p = contact.points[index];
    Vec2 r1 = p - contact.obj1->transform.position;
    Vec2 r2 = p - contact.obj2->transform.position;

    Vec2 normal = contact.normal;
    Vec2 tangent = Vec2(-normal.y, normal.x);

    Vec2 impulse = normal * jn + tangent * jt;

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