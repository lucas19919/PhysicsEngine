#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/physics/ManifoldHandler.h"
#include "main/physics/Resolve.h"

World::World() : spatialHash(100.0f)
{
    gravity = Vec2(0.0f, 600.0f);
}

void World::AddGameObject(std::unique_ptr<GameObject> obj)
{
    gameObjects.push_back(std::move(obj));
}

std::vector<std::unique_ptr<GameObject>>& World::GetGameObjects()
{
    return gameObjects;
}

const std::vector<std::unique_ptr<GameObject>>& World::GetGameObjects() const
{
    return gameObjects;
}

void World::Clear()
{
    GetGameObjects().clear();
    gridMap.clear();
}

void World::Step(float dt)
{
    if (isPaused) return;

    gridMap.clear();

    for (const auto& objPtr : GetGameObjects())
    {
        GameObject* obj = objPtr.get();
        Collider* c = obj->GetCollider();
        RigidBody *rb = obj->GetRigidBody();

        BBox bounds = spatialHash.GetBounding(obj);

        int minX = std::floor(bounds.min.x / spatialHash.GetCellSize());
        int maxX = std::floor(bounds.max.x / spatialHash.GetCellSize());
        int minY = std::floor(bounds.min.y / spatialHash.GetCellSize());
        int maxY = std::floor(bounds.max.y / spatialHash.GetCellSize());

        for (int x = minX; x <= maxX; x++)
        {
            for (int y = minY; y <= maxY; y++)
            {
                unsigned int hash = spatialHash.GetHash(Vec2(x * spatialHash.GetCellSize(), y * spatialHash.GetCellSize()));
                gridMap[hash].push_back(obj);
            }
        }

        if (rb == nullptr) continue;
        if (rb->isSleeping) continue;

        float iM = rb->GetInvMass();
        float M = rb->GetMass();
        
        float I = rb->GetInertia();
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

void World::CheckCollisions()
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

    int solverSubTicks = 6;

    for (int i = 0; i < solverSubTicks; i++)
    {
        for (const auto& collisionPair : collisionPairs)
        {
            GameObject* obj1 = collisionPair.first;
            GameObject* obj2 = collisionPair.second;

            CollisionManifold cm = Resolve::ResolveManifold(obj1, obj2);
            if (cm.Collision.isColliding)
            {
                Resolve::ResolvePosition(cm, obj1, obj2);
                Resolve::ResolveImpulse(cm, obj1, obj2);
            }                
        }
    }
}