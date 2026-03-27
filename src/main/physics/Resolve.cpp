#include "main/physics/Resolve.h"
#include "main/GameObject.h"

CollisionManifold Resolve::ResolveManifold(GameObject* obj1, GameObject* obj2)
{
    return ManifoldHandler::SortManifold(obj1, obj2);
}

void Resolve::ResolveImpulse(CollisionManifold manifold, GameObject* obj1, GameObject* obj2)
{
    RigidBody *rb1 = obj1->GetRigidBody();
    RigidBody *rb2 = obj2->GetRigidBody();

    float invMass1 = rb1 ? rb1->GetInvMass() : 0;
    float invMass2 = rb2 ? rb2->GetInvMass() : 0;
    float invInertia1 = rb1 ? rb1->GetInvInertia() : 0;
    float invInertia2 = rb2 ? rb2->GetInvInertia() : 0;

    if (invMass1 == 0.0f && invMass2 == 0.0f) return;

    float e = 0.0f;
    if (rb1 && rb2)
        e = std::min(rb1->GetRestitution(), rb2->GetRestitution());
    else if (rb1)
        e = rb1->GetRestitution();
    else if (rb2)
        e = rb2->GetRestitution();

    if (manifold.points.size() == 0) return;
    
    for (Vec2 contact : manifold.points)
    {
        Vec2 normal = manifold.Collision.normal;

        Vec2 r1 = contact - obj1->transform.position;
        Vec2 r2 = contact - obj2->transform.position;

        Vec2 v1 = rb1 ? rb1->velocity : Vec2(0,0);
        Vec2 v2 = rb2 ? rb2->velocity : Vec2(0,0);
        float w1 = rb1 ? rb1->angularVelocity : 0;
        float w2 = rb2 ? rb2->angularVelocity : 0;

        Vec2 totalV1 = v1 + Vec2(-w1 * r1.y, w1 * r1.x);
        Vec2 totalV2 = v2 + Vec2(-w2 * r2.y, w2 * r2.x);

        Vec2 relative = totalV2 - totalV1;
        float magnitude = normal.Dot(relative);
        
        if (magnitude > 0.0f) continue;

        float cross1 = r1.Cross(normal) * r1.Cross(normal);
        float cross2 = r1.Cross(normal) * r1.Cross(normal);

        float j = (-(1 + e) * magnitude) / (invMass1 + invMass2 + (cross1 * invInertia1) + (cross2 * invInertia2));
        j /= manifold.points.size();

        Vec2 impulse = normal * j;

        if (rb1)
        {
            rb1->velocity = rb1->velocity - impulse * invMass1;
            rb1->angularVelocity -= (r1.x * impulse.y - r1.y * impulse.x) * invInertia1;
        }
        if (rb2)
        {
            rb2->velocity = rb2->velocity + impulse * invMass2;
            rb2->angularVelocity += (r2.x * impulse.y - r2.y * impulse.x) * invInertia2;
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