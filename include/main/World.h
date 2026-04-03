#pragma once
#include "main/GameObject.h"
#include <vector>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <set>
#include "main/physics/SpatialHash.h"

struct ContactConstraint
{
    unsigned int key;

    GameObject* obj1;
    GameObject* obj2;
    RigidBody* rb1;
    RigidBody* rb2;

    Vec2 normal;
    float penetration;

    Array<20> points;
    int pointCount;

    float restitution;
    float restitutionBias[2] = {0.0f, 0.0f};

    float friction;

    float accumulatedNormalImpulse[20];
    float accumulatedTangentImpulse[20];
};

class World
{
    public:
        World();
        
        void Step(float dt);
        void Clear();       

        void AddGameObject(std::unique_ptr<GameObject> obj);
        std::vector<std::unique_ptr<GameObject>>& GetGameObjects();

        //abstract stuff like this later ???
        bool isPaused = true;

    private:
        void PrepareFrame(float dt);
        void IntegrateVelocities(float dt);

        void UpdateBroadphase();
        void GeneratePairs();
        void BuildContacts();

        void PrepareContacts();
        void SolveConstraints();
        void IntegratePositions(float dt);

        void FinishFrame(float dt);
        void UpdateSleep(float dt);

    private:
        SpatialHash spatialHash;
        std::unordered_map<unsigned int, std::vector<GameObject*>> gridMap;
        std::vector<std::pair<GameObject*, GameObject*>> candidatePairs;

        std::vector<std::unique_ptr<GameObject>> gameObjects;

        std::vector<ContactConstraint> currentFrameContacts;
        std::vector<ContactConstraint> lastFrameContacts;
};