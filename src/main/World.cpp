#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/physics/CollisionHandler.h"

World::World()
{
    gravity = Vec2(0.0f, 981.0f);
    airDensity = 0.001f;
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

void World::CheckCollisons(int screenWidth, int screenHeight)
{
    for (int i = 0; i < GetGameObjects().size(); i++)
    {
        GameObject* obj = GetGameObjects()[i];
        Collider* c = obj->GetCollider();
        if (c == nullptr) continue;

        for (int j = i + 1; j < GetGameObjects().size(); j++)
        {
            GameObject* other = GetGameObjects()[j];
            Collider* otherCol = other->GetCollider();
            if (otherCol == nullptr) continue;

            Manifold collision = CollisionHandler::SortCollision(c, otherCol);
            if (collision.isColliding) {
                RigidBody* rb = obj->GetRigidBody();
                RigidBody* otherRb = other->GetRigidBody();

                float invMassA = (rb != nullptr) ? rb->GetInvMass() : 0.0f;
                float invMassB = (otherRb != nullptr) ? otherRb->GetInvMass() : 0.0f;
                float totalInvMass = invMassA + invMassB;

                if (totalInvMass == 0.0f) continue;

                Vec2 correction = collision.normal * (collision.depth / totalInvMass);
                
                obj->transform.position = obj->transform.position - (correction * invMassA);
                other->transform.position = other->transform.position + (correction * invMassB);

                if (rb != nullptr && otherRb != nullptr) {
                    Vec2 relV = otherRb->velocity - rb->velocity;
                    float relVNormal = relV.Dot(collision.normal);

                    if (relVNormal > 0.0f) continue;

                    float e = std::min(rb->GetRestitution(), otherRb->GetRestitution());
                    float j = -(1 + e) * relVNormal;
                    j /= totalInvMass;

                    Vec2 impulse = collision.normal * j;
                    rb->velocity = rb->velocity - (impulse * invMassA);
                    otherRb->velocity = otherRb->velocity + (impulse * invMassB);
                }
                else
                {
                    if (rb != nullptr) {
                        rb->velocity = rb->velocity - collision.normal * (1 + rb->GetRestitution()) * rb->velocity.Dot(collision.normal);
                    }
                    if (otherRb != nullptr) {
                        otherRb->velocity = otherRb->velocity - (collision.normal * -1.0f) * (1 + otherRb->GetRestitution()) * otherRb->velocity.Dot(collision.normal * -1.0f);
                    }                    
                }
            }
        }
    }
}