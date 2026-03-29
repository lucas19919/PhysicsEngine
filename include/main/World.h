#pragma once
#include "main/GameObject.h"
#include <vector>
#include <algorithm>
#include <memory>

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

    private:
        std::vector<std::unique_ptr<GameObject>> gameObjects;
};