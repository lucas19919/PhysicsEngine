#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/physics/ManifoldHandler.h"
#include "main/physics/Resolve.h"
#include <unordered_set>

World::World() : spatialHash(35.0f)
{
    gravity = Vec2(0.0f, 600.0f);
}

void World::Clear()
{
    gameObjects.clear();
    gridMap.clear();
    collisionPairs.clear();
}

void World::Step(float dt)
{
    if (isPaused) return;

    Integrate(dt);
    UpdateGrid();
    GenerateCollisionPairs();

    int subTicks = 8;
    for (int i = 0; i < subTicks; i++)
    {
        ResolveCollisions();
    }
}

void World::Integrate(float dt)
{
    sleepCounter = 0;
    for (const auto& objPtr : gameObjects)
    {
        GameObject* obj = objPtr.get();
        RigidBody* rb = obj->GetRigidBody();

        if (!rb) continue;
        rb->UpdateSleep(dt);
        rb->ResetContacts();

        if (rb->isSleeping) 
        {
            rb->SetVelocity(Vec2(0, 0));
            rb->SetAngularVelocity(0.0f);
            sleepCounter++;
            continue;
        }
        
        float M = rb->GetMass();
        float iM = rb->GetInvMass();
        float iI = rb->GetInvInertia();

        rb->ApplyForce(gravity * M);
        
        rb->SetAcceleration(rb->GetForce() * iM);
        rb->SetVelocity(rb->GetVelocity() + rb->GetAcceleration() * dt);

        obj->transform.position += rb->GetVelocity() * dt;

        rb->SetAngularAcceleration(rb->GetTorque() * iI);
        rb->SetAngularVelocity(rb->GetAngularVelocity() + rb->GetAngularAcceleration() * dt);

        obj->transform.rotation += rb->GetAngularVelocity() * dt;

        rb->ClearTorque();
        rb->ClearForces();
    }
}

void World::UpdateGrid()
{
    for (auto& pair : gridMap)  
    {
        pair.second.clear();
    }

    for (const auto& objPtr : gameObjects)
    {
        GameObject* obj = objPtr.get();
        Collider* c = obj->GetCollider();
        RigidBody* rb = obj->GetRigidBody();

        if (rb == nullptr || !rb->isSleeping)
        {
            obj->cachedVertices = SAT::GetVertices(obj);
            obj->cachedNormals = SAT::GetNormals(obj->cachedVertices);
            c->SetBounds(spatialHash.GetBounding(obj));
        }
        int minX = std::floor(c->GetBounds().min.x / spatialHash.GetCellSize());
        int maxX = std::floor(c->GetBounds().max.x / spatialHash.GetCellSize());
        int minY = std::floor(c->GetBounds().min.y / spatialHash.GetCellSize());
        int maxY = std::floor(c->GetBounds().max.y / spatialHash.GetCellSize());

        for (int x = minX; x <= maxX; x++)
        {
            for (int y = minY; y <= maxY; y++)
            {
                unsigned int hash = spatialHash.GetHash(Vec2(x * spatialHash.GetCellSize(), y * spatialHash.GetCellSize()));
                gridMap[hash].push_back(obj);
            }
        }
    }
}

void World::GenerateCollisionPairs()
{
    collisionPairs.clear();
    collisionPairs.reserve(gameObjects.size() * 2);

    std::unordered_set<std::pair<GameObject*, GameObject*>, PairHash> seen;

    for (auto& pair : gridMap)
    {
        std::vector<GameObject*>& cell = pair.second;
        if (cell.size() < 2) continue;     

        for (int i = 0; i < cell.size(); i++)
        {
            for (int j = i + 1; j < cell.size(); j++)    
            {
                GameObject* obj1 = cell[i];
                GameObject* obj2 = cell[j];

                if (obj1 > obj2) std::swap(obj1, obj2);

                size_t pairHash = (reinterpret_cast<size_t>(obj1) << 32) | (reinterpret_cast<size_t>(obj2) & 0xFFFFFFFF);

                if (seen.insert({obj1, obj2}).second) 
                {
                    collisionPairs.push_back({obj1, obj2}); 
                }
            }
        }
    }
}

void World::ResolveCollisions()
{
    for (const auto& collisionPair : collisionPairs)
    {
        GameObject* obj1 = collisionPair.first;
        GameObject* obj2 = collisionPair.second;

        RigidBody* rb1 = obj1->GetRigidBody();
        RigidBody* rb2 = obj2->GetRigidBody();

        CollisionManifold cm = Resolve::ResolveManifold(obj1, obj2);
        if (cm.Collision.isColliding)
        {
            if (rb1) rb1->AddContact(cm.Collision.normal);
            if (rb2) rb2->AddContact(Vec2(-cm.Collision.normal.x, -cm.Collision.normal.y));
            
            Vec2 v1 = rb1 ? rb1->GetVelocity() : Vec2(0,0);
            Vec2 v2 = rb2 ? rb2->GetVelocity() : Vec2(0,0);
            float relativeVel = (v2 - v1).Dot(cm.Collision.normal);

            if (std::abs(relativeVel) > 10.0f) 
            {
                if (rb1 && rb1->isSleeping) rb1->WakeUp();
                if (rb2 && rb2->isSleeping) rb2->WakeUp();
            }

            obj1->cachedVertices = SAT::GetVertices(obj1);
            obj1->cachedNormals = SAT::GetNormals(obj1->cachedVertices);
            obj2->cachedVertices = SAT::GetVertices(obj2);
            obj2->cachedNormals = SAT::GetNormals(obj2->cachedVertices);

            Resolve::ResolvePosition(cm, obj1, obj2);
            Resolve::ResolveImpulse(cm, obj1, obj2);
        }                
    }
}