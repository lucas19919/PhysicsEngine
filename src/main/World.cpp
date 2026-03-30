#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/physics/ManifoldHandler.h"
#include "main/physics/Resolve.h"
#include <cstdio>

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
    ResolveCollisions();    
}

void World::Integrate(float dt)
{
    int isAsleep = 0;
    for (const auto& objPtr : gameObjects)
    {
        GameObject* obj = objPtr.get();
        RigidBody* rb = obj->GetRigidBody();

        if (rb == nullptr) continue;
        if (rb->isSleeping) isAsleep++;

        rb->UpdateSleep(dt);
        if (rb->isSleeping) continue;

        float iM = rb->GetInvMass();
        float M = rb->GetMass();
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

    printf("Sleeping: %d / %d\n", isAsleep, (int)gameObjects.size());
}

void World::UpdateGrid()
{
    gridMap.clear();

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
                
                collisionPairs.push_back({obj1, obj2}); 
            }
        }
    }
    
    std::sort(collisionPairs.begin(), collisionPairs.end());
    collisionPairs.erase(std::unique(collisionPairs.begin(), collisionPairs.end()), collisionPairs.end());
}

void World::ResolveCollisions()
{
    for (const auto& collisionPair : collisionPairs)
    {
        GameObject* obj1 = collisionPair.first;
        GameObject* obj2 = collisionPair.second;

        RigidBody* rb1 = obj1->GetRigidBody();
        RigidBody* rb2 = obj2->GetRigidBody();

        bool isObj1Static = (rb1 == nullptr || rb1->GetMass() == 0.0f);
        bool isObj2Static = (rb2 == nullptr || rb2->GetMass() == 0.0f);

        if (!isObj1Static && !isObj2Static && rb1->isSleeping && rb2->isSleeping) continue;
        if (!isObj1Static && isObj2Static && rb1->isSleeping) continue;
        if (isObj1Static && !isObj2Static && rb2->isSleeping) continue;

        CollisionManifold cm = Resolve::ResolveManifold(obj1, obj2);
        if (cm.Collision.isColliding)
        {
            if (rb1 && rb1->isSleeping) rb1->WakeUp();
            if (rb2 && rb2->isSleeping) rb2->WakeUp();

            Resolve::ResolvePosition(cm, obj1, obj2);
            Resolve::ResolveImpulse(cm, obj1, obj2);
        }                
    }
}