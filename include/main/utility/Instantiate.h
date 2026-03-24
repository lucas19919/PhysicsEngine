#pragma once
#include "main/GameObject.h"
#include "main/World.h"
#include <vector>
#include <variant>

class Instantiate
{
    public:
        Instantiate();

        Instantiate& WithRigidBody(Properties properties, LinearState linearState, AngularState angularState);
        Instantiate& WithRenderer(Shape shape);
        Instantiate& WithCollider(ColliderType type, std::variant<Vec2, float, std::vector<Vec2>> bounds);
        Instantiate& WithTransform(Vec2 position, float rotation);

        GameObject* Create(World& world);
    private:
        GameObject* obj;
};