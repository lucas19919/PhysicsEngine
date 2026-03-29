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
    
    void AddGameObject(std::unique_ptr<GameObject> obj);

    std::vector<std::unique_ptr<GameObject>>& GetGameObjects();
    const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const;

    void Clear();        
    void Step(float dt);
    void CheckCollisions();

    Vec2 gravity;
    bool isPaused = true;

    SpatialHash spatialHash;
    std::unordered_map<unsigned int, std::vector<GameObject*>> gridMap;

private:
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::vector<std::pair<GameObject*, GameObject*>> collisionPairs;
};