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

        std::vector<Vec2> bounds = spatialHash.GetBounding(obj);

        int minX = std::floor(bounds[0].x / spatialHash.GetCellSize());
        int maxX = std::floor(bounds[1].x / spatialHash.GetCellSize());
        int minY = std::floor(bounds[0].y / spatialHash.GetCellSize());
        int maxY = std::floor(bounds[1].y / spatialHash.GetCellSize());

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
    std::set<std::pair<GameObject*, GameObject*>> checked;

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
                std::pair<GameObject*, GameObject*> collisionPair = {obj1, obj2};

                if (checked.find(collisionPair) != checked.end()) continue;
                checked.insert(collisionPair);

                CollisionManifold cm = Resolve::ResolveManifold(obj1, obj2);
                if (cm.Collision.isColliding == true)
                {
                    Resolve::ResolvePosition(cm, obj1, obj2);
                    Resolve::ResolveImpulse(cm, obj1, obj2);
                }                
            }
        }
    }










}