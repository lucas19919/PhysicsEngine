#pragma once
#include "main/GameObject.h"
#include "main/physics/SpatialHash.h"
#include "main/components/Constraint.h"
#include "main/components/Controller.h"
#include <vector>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <cstdint>

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
    float restitutionBias[20] = {};

    float friction;

    float accumulatedNormalImpulse[20] = {};
    float accumulatedTangentImpulse[20] = {};
};

class World
{
    public:
        World();
        
        void Step(float dt);
        void Clear();       

        void AddGameObject(std::unique_ptr<GameObject> obj);
        std::vector<std::unique_ptr<GameObject>>& GetGameObjects();

        void AddConstraint(std::unique_ptr<Constraint> c);
        const std::vector<std::unique_ptr<Constraint>>& GetConstraints() const { return constraints; }

        void AddController(std::unique_ptr<Controller> c) { controllers.push_back(std::move(c)); }
        const std::vector<std::unique_ptr<Controller>>& GetControllers() const { return controllers; }

        //abstract stuff like this later ???
        bool isPaused = true;

    private:
        void PrepareFrame(float dt);
        void IntegrateVelocities(float dt);

        void UpdateBroadphase();
        void GeneratePairs();

        void BuildContacts();
        void PrepareContacts(float dt);
        void SolveConstraints(float dt);

        void IntegratePositions(float dt);

        void FinishFrame(float dt);

    private:
        SpatialHash spatialHash;
        std::unordered_map<unsigned int, std::vector<GameObject*>> gridMap;

        std::vector<std::pair<GameObject*, GameObject*>> candidatePairs;
        std::unordered_set<uint64_t> candidatePairKeys;

        std::vector<std::unique_ptr<GameObject>> gameObjects;

        std::vector<ContactConstraint> currentFrameContacts;
        std::vector<ContactConstraint> lastFrameContacts;

        std::vector<std::unique_ptr<Constraint>> constraints;
        
        std::vector<std::unique_ptr<Controller>> controllers;
};