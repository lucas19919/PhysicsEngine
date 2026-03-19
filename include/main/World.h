#pragma once
#include "main/GameObject.h"
#include <vector>
#include <algorithm>

class World
{
    public:
        World();
        ~World();
        
        std::vector<GameObject*> GetGameObjects() const { return gameObjects; }
        void AddGameObject(GameObject* obj) { gameObjects.push_back(obj); }
        void removeGameObject(GameObject* obj) 
        { 
            gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), obj), gameObjects.end());
            delete obj;
        }

        void Step(float dt);
        void CheckCollisons(int screenWidth, int screenHeight);

        Vec2 gravity;
        float airDensity;

        private:
            std::vector<GameObject*> gameObjects;
};