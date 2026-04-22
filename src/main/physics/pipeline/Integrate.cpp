#include "main/physics/pipeline/Integrate.h"
#include "main/components/RigidBody.h"
#include "main/physics/Config.h"

void Integrate::IntegrateVelocity(std::vector<std::unique_ptr<GameObject>>& gameObjects, float dt)
{
    for (const auto& objPtr : gameObjects)
    {
        GameObject* obj = objPtr.get();
        RigidBody* rb = obj->rb;   

        if (!rb) continue;

        float mass = rb->GetMass();
        float invMass = rb->GetInvMass();
        float invInertia = rb->GetInvInertia();

        //linear motion
        if (rb->IsGravityEnabled())
            rb->ApplyForce(Config::gravity * mass);

        rb->SetAcceleration(rb->GetForce() * invMass);
        rb->SetVelocity(rb->GetVelocity() + rb->GetAcceleration() * dt);

        //angular motion
        rb->SetAngularAcceleration(rb->GetTorque() * invInertia);
        rb->SetAngularVelocity(rb->GetAngularVelocity() + rb->GetAngularAcceleration() * dt);

        rb->ClearForces();
        rb->ClearTorque();
    }
}

void Integrate::IntegratePosition(std::vector<std::unique_ptr<GameObject>>& gameObjects, float dt)
{
    for (const auto& objPtr : gameObjects)
    {
        GameObject* obj = objPtr.get();
        RigidBody* rb = obj->rb;   

        if (!rb) continue;

        obj->transform.position += rb->GetVelocity() * dt;
        obj->transform.rotation += rb->GetAngularVelocity() * dt;
        obj->transform.isDirty = true;
    }
}