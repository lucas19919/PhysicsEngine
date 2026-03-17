#pragma once
#include <vector>
#include "RigidBody.h"

class World
{
    public:
        Vec2 gravity;
        float airDensity;

        World();
        ~World();

        void AddBody(RigidBody* body);
        void Step(float dt);
        void CheckCollisons(int screenWidth, int screenHeight);

        std::vector<RigidBody*> bodies;
};