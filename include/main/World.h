#pragma once
#include "main/GameObject.h"
#include <vector>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <set>
#include "main/physics/SpatialHash.h"

class World
{
public:
    World();

    void Clear();       
    void Step(float dt);

    void AddGameObject(std::unique_ptr<GameObject> obj) { GetGameObjects().push_back(std::move(obj)); }
    std::vector<std::unique_ptr<GameObject>>& GetGameObjects() { return gameObjects; }
 
    Vec2 gravity;
    bool isPaused = true;

private:
    void Integrate(float dt);
    void UpdateGrid();
    void GenerateCollisionPairs();
    void ResolveCollisions();

    std::vector<std::unique_ptr<GameObject>> gameObjects;

    SpatialHash spatialHash;
    std::unordered_map<unsigned int, std::vector<GameObject*>> gridMap;
    std::vector<std::pair<GameObject*, GameObject*>> collisionPairs;
};