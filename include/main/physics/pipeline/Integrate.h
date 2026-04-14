#pragma once
#include "main/GameObject.h"
#include <vector>
#include <memory>

class GameObject;

class Integrate
{
    public:
        static void IntegrateVelocity(std::vector<std::unique_ptr<GameObject>>& gameObjects, float dt);
        static void IntegratePosition(std::vector<std::unique_ptr<GameObject>>& gameObjects, float dt);
};