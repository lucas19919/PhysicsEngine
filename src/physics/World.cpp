#include "physics/World.h"
#include "physics/RigidBody.h"
#include "math/Vec2.h"

World::World()
{
    gravity = Vec2(0.0f, 981.0f);
    airDensity = 0.001f;
}

World::~World()
{
    for (RigidBody* rb : bodies) {
        delete rb;
    }
    bodies.clear();
}

void World::AddBody(RigidBody* body)
{
    bodies.push_back(body);
}

void World::Step(float dt)
{
    for (RigidBody *rb : bodies)
    {
        float iM = rb->GetInvMass();
        float M = rb->GetMass();

        rb->ApplyForce(gravity * M);
        float speedSq = rb->velocity.MagSq();
        if (speedSq > 0.0001f)
        {
            float drag = airDensity * speedSq * rb->GetRadius();
            rb->ApplyForce(rb->velocity.Norm() * -1.0f * drag);
        }
       
        rb->acceleration = rb->GetForce() * iM;
        rb->velocity += rb->acceleration * dt;
        rb->position += rb->velocity * dt;

        rb->ClearForces();
    }
}

void World::CheckCollisons(int screenWidth, int screenHeight)
{
    for (int i = 0; i < bodies.size(); i++)
    {
        RigidBody *rb = bodies[i];
    
        float radius = rb->GetRadius();
        if (rb->position.x + radius > screenWidth)
        {
            rb->position.x = screenWidth - radius;
            rb->velocity.x *= -1; 
        }
        
        if (rb->position.x - radius < 0)
        {
            rb->position.x = radius;
            rb->velocity.x *= -1; 
        }

        if (rb->position.y + radius > screenHeight)
        {
            rb->position.y = screenHeight - radius;
            rb->velocity.y *= -1; 
        }
        
        if (rb->position.y - radius < 0)
        {
            rb->position.y = radius;
            rb->velocity.y *= -1; 
        }

        for (int j = i + 1; j < bodies.size(); j++)
        {
            RigidBody *rbB = bodies[j];

            Vec2 dir = rbB->position - rb->position;
            if (dir.MagSq() < (rbB->GetRadius() + rb->GetRadius()) * (rbB->GetRadius() + rb->GetRadius()))
            {
                Vec2 dirN = dir.Norm();
                Vec2 relativeV = rbB->velocity - rb->velocity;
                float dot = relativeV.Dot(dirN);
                if (dot > 0) continue;

                float bounce = (rb->GetRes() + rbB->GetRes()) / 2.0f; 
                float impulse = -1 * (1 + bounce) * dot / (rb->GetInvMass() + rbB->GetInvMass());
                Vec2 impulseVec = dirN * impulse;

                rb->velocity += impulseVec * rb->GetInvMass() * -1;
                rbB->velocity += impulseVec * rbB->GetInvMass();

                float overlap = (rb->GetRadius() + rbB->GetRadius()) - dir.Mag();
                Vec2 correction = dirN * (overlap / (rb->GetInvMass() + rbB->GetInvMass())) * 0.8f; 

                rb->position += correction * rb->GetInvMass() * -1;
                rbB->position += correction * rbB->GetInvMass();
            }
        }
    }
}