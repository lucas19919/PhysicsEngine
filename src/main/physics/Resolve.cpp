#include "main/physics/Resolve.h"
#include "main/GameObject.h"

CollisionManifold Resolve::ResolveManifold(GameObject* obj1, GameObject* obj2)
{
    return ManifoldHandler::SortManifold(obj1, obj2);
}

void Resolve::ResolveImpulse(CollisionManifold manifold, GameObject* obj1, GameObject* obj2)
{
    if (manifold.points.size() == 0) return;
    
    RigidBody *rb1 = obj1->GetRigidBody();
    RigidBody *rb2 = obj2->GetRigidBody();

    float invMass1 = rb1 ? rb1->GetInvMass() : 0;
    float invMass2 = rb2 ? rb2->GetInvMass() : 0;
    float invInertia1 = rb1 ? rb1->GetInvInertia() : 0;
    float invInertia2 = rb2 ? rb2->GetInvInertia() : 0;

    if (invMass1 == 0.0f && invMass2 == 0.0f) return;

    float e = 0.0f;
    float f = 0.0f;

    if (rb1 && rb2)
    {
        e = std::min(rb1->GetRestitution(), rb2->GetRestitution());
        f = std::sqrt(rb1->GetFriction() * rb2->GetFriction());
    }
    else if (rb1)
    {
        e = rb1->GetRestitution();
        f = rb1->GetFriction();
    }
    else if (rb2)
    {
        e = rb2->GetRestitution();
        f = rb2->GetFriction();
    }
    
    for (Vec2 contact : manifold.points)
    {
        //j impulse
        Vec2 normal = manifold.Collision.normal;

        Vec2 r1 = contact - obj1->transform.position;
        Vec2 r2 = contact - obj2->transform.position;

        Vec2 v1 = rb1 ? rb1->GetVelocity() : Vec2(0,0);
        Vec2 v2 = rb2 ? rb2->GetVelocity() : Vec2(0,0);
        float w1 = rb1 ? rb1->GetAngularVelocity() : 0;
        float w2 = rb2 ? rb2->GetAngularVelocity() : 0;

        Vec2 totalV1 = v1 + Vec2(-w1 * r1.y, w1 * r1.x);
        Vec2 totalV2 = v2 + Vec2(-w2 * r2.y, w2 * r2.x);

        Vec2 relative = totalV2 - totalV1;
        float magnitude = normal.Dot(relative);
        
        if (magnitude > 0.0f) continue;

        float cross1 = r1.Cross(normal);
        float cross2 = r2.Cross(normal);

        float j = (-(1 + e) * magnitude) / (invMass1 + invMass2 + (cross1 * cross1 * invInertia1) + (cross2 * cross2 * invInertia2));
        j /= manifold.points.size();

        Vec2 impulse = normal * j;

        if (rb1)
        {
            rb1->SetVelocity(rb1->GetVelocity() - impulse * invMass1);
            rb1->SetAngularVelocity(rb1->GetAngularVelocity() - (r1.x * impulse.y - r1.y * impulse.x) * invInertia1);
        }
        if (rb2)
        {
            rb2->SetVelocity(rb2->GetVelocity() + impulse * invMass2);
            rb2->SetAngularVelocity(rb2->GetAngularVelocity() + (r2.x * impulse.y - r2.y * impulse.x) * invInertia2);
        }

        //friction impulse
        Vec2 tangent = relative - (normal * relative.Dot(normal));
        
        if (tangent.MagSq() > 0.0001f)
            tangent = tangent.Norm();
        else continue;

        float crossT1 = r1.Cross(tangent);
        float crossT2 = r2.Cross(tangent);

        float jt = -relative.Dot(tangent) / (invMass1 + invMass2 + (crossT1 * crossT1 * invInertia1) + (crossT2 * crossT2 * invInertia2));
        jt /= manifold.points.size();

        float jtMag = 0.0f;
        
        if (std::abs(jt) <= j * f)
            jtMag = jt;
        else
            jtMag = (jt > 0.0f ? 1.0f : -1.0f) * j * f;

        Vec2 frictionImpulse = tangent * jtMag;

        if (rb1)
        {
            rb1->SetVelocity(rb1->GetVelocity() - frictionImpulse * invMass1);
            rb1->SetAngularVelocity(rb1->GetAngularVelocity() - (r1.x * frictionImpulse.y - r1.y * frictionImpulse.x) * invInertia1);
        }
        if (rb2)
        {
            rb2->SetVelocity(rb2->GetVelocity() + frictionImpulse * invMass2);
            rb2->SetAngularVelocity(rb2->GetAngularVelocity() + (r2.x * frictionImpulse.y - r2.y * frictionImpulse.x) * invInertia2);
        }
    }
}

void Resolve::ResolvePosition(CollisionManifold manifold, GameObject* obj1, GameObject* obj2)
{

    RigidBody* rb1 = obj1->GetRigidBody();
    RigidBody* rb2 = obj2->GetRigidBody();

    float invMassA = rb1 ? rb1->GetInvMass() : 0.0f;
    float invMassB = rb2 ? rb2->GetInvMass() : 0.0f;
    float totalInvMass = invMassA + invMassB;

    if (totalInvMass == 0.0f) return;

    Collision collision = manifold.Collision;

    float slop = 0.05f;

    float penetration = std::max(collision.depth - slop, 0.0f);
    Vec2 correction = collision.normal * (collision.depth / totalInvMass);
    
    if (rb1)
    {
        obj1->transform.position = obj1->transform.position - (correction * invMassA);
    }
    if (rb2)
    {
        obj2->transform.position = obj2->transform.position + (correction * invMassB);
    }
}