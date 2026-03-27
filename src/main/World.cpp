#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/physics/ManifoldHandler.h"
#include "main/physics/Resolve.h"

World::World()
{
    gravity = Vec2(0.0f, 600.0f);
}

void World::Clear()
{
    for (GameObject* obj : GetGameObjects()) {
        delete obj;
    }
    
    GetGameObjects().clear();
}

void World::Step(float dt)
{
    for (GameObject *obj : GetGameObjects())
    {
        RigidBody *rb = obj->GetRigidBody();
        Collider *c = obj->GetCollider();
        if (rb == nullptr) continue;

        float iM = rb->GetInvMass();
        float M = rb->GetMass();
        
        float I = rb->GetInertia();
        float iI = rb->GetInvInertia();

        rb->ApplyForce(gravity * M);
        
        rb->acceleration = rb->GetForce() * iM;
        rb->velocity += rb->acceleration * dt;

        obj->transform.position += rb->velocity * dt;

        rb->angularAcceleration = rb->GetTorque() * iI;
        rb->angularVelocity += rb->angularAcceleration * dt;

        obj->transform.rotation += rb->angularVelocity * dt;

        rb->ClearTorque();
        rb->ClearForces();
    }
}

void World::CheckCollisions()
{
    for (int i = 0; i < GetGameObjects().size(); i++)
    {
        GameObject* obj = GetGameObjects()[i];

        for (int j = i + 1; j < GetGameObjects().size(); j++)
        {
            GameObject* obj2 = GetGameObjects()[j];
            CollisionManifold cm = Resolve::ResolveManifold(obj, obj2);
            if (cm.Collision.isColliding == true)
            {
                Resolve::ResolvePosition(cm, obj, obj2);
                Resolve::ResolveImpulse(cm, obj, obj2);
            }
        }
    }
}