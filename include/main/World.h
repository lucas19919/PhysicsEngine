#pragma once
#include "main/GameObject.h"
#include <vector>
#include <algorithm>

class World
{
    public:
        World();
        void Clear();
        
        std::vector<GameObject*> GetGameObjects() const { return gameObjects; }
        void AddGameObject(GameObject* obj) { gameObjects.push_back(obj); }
        void RemoveGameObject(GameObject* obj) 
        { 
            gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), obj), gameObjects.end());
            delete obj;
        }

        void Step(float dt);
        void CheckCollisions();

        Vec2 gravity;
        bool isPaused = true;

    private:
        std::vector<GameObject*> gameObjects;
};