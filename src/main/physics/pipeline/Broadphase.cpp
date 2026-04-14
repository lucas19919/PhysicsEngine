#include "main/physics/pipeline/Broadphase.h"
#include "main/physics/SAT.h"
#include <algorithm>

void Broadphase::Clear()
{
    gridMap.clear();
    candidatePairKeys.clear();
    gridMap.clear();
}

void Broadphase::UpdateBroadphase(std::vector<std::unique_ptr<GameObject>>& gameObjects)
{
    candidatePairKeys.clear();
    for (auto& cell : gridMap)
    {
        cell.second.clear();
    }

    for (const auto& objPtr : gameObjects)
    {
        GameObject* obj = objPtr.get();
        RigidBody* rb = obj->GetRigidBody();
        Collider* c = obj->GetCollider();

        if (!c) continue;

        obj->cachedVertices = SAT::GetVertices(obj);
        obj->cachedNormals = SAT::GetNormals(obj->cachedVertices);
        c->SetBounds(spatialHash.GetBounding(obj));

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

std::vector<std::pair<GameObject*, GameObject*>> Broadphase::GeneratePairs()
{
    std::vector<std::pair<GameObject*, GameObject*>> candidatePairs;

    //for each cell, generate pairs if they overlap bounds
    for (const auto& pair : gridMap)
    {
        const std::vector<GameObject*>& cell = pair.second;
        if (cell.size() < 2) continue; //need at least 2 objects to form a pair

        for (size_t i = 0; i < cell.size(); i++)
        {
            for (size_t j = i + 1; j < cell.size(); j++)
            {
                GameObject* obj1 = cell[i];
                GameObject* obj2 = cell[j];

                size_t obj1ID = obj1->GetID();
                size_t obj2ID = obj2->GetID();

                if (std::find(obj1->GetIgnoredIDs().begin(), obj1->GetIgnoredIDs().end(), obj2ID) != obj1->GetIgnoredIDs().end() ||
                    std::find(obj2->GetIgnoredIDs().begin(), obj2->GetIgnoredIDs().end(), obj1ID) != obj2->GetIgnoredIDs().end())
                    continue;

                RigidBody* rb1 = obj1->GetRigidBody();
                RigidBody* rb2 = obj2->GetRigidBody();

                //check if bounds overlap before adding pair
                if (SAT::TestBounds(obj1, obj2))
                {
                    if (obj1 > obj2) std::swap(obj1, obj2); 

                    uint64_t key = (uint64_t)(uintptr_t)obj1 ^ ((uint64_t)(uintptr_t)obj2 << 32);
                    if (!candidatePairKeys.insert(key).second) continue;

                    candidatePairs.emplace_back(obj1, obj2);
                }
            }
        }
    }
    
    return candidatePairs;
}