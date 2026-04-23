#pragma once
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/components/RigidBody.h"
#include "main/utility/templates/Array.h"
#include "main/components/Constraint.h"

#include <vector>
#include <memory>
#include <unordered_map>

struct ImpulseCache
{
    float normalImpulse[4];
    float tangentImpulse[4];
    int pointCount;
};

struct ContactConstraint
{
    unsigned int key;

    GameObject* obj1;
    GameObject* obj2;
    RigidBody* rb1;
    RigidBody* rb2;

    Vec2 normal;
    float penetration;

    Array<4> points;
    int pointCount;

    Vec2 r1[4];
    Vec2 r2[4];
    float normalMass[4];
    float tangentMass[4];

    float restitution;
    float restitutionBias[4] = {};

    float friction;

    float accumulatedNormalImpulse[4] = {};
    float accumulatedTangentImpulse[4] = {};
};

class ContactManager
{
    public: 
        void PrepareFrame();
        void FinishFrame();
        void Clear();

        void BuildContacts(std::vector<std::pair<GameObject*, GameObject*>> candidatePairs);
        void PrepareContacts(float dt);
        void SolveConstraints(const std::vector<std::unique_ptr<Constraint>>& constraints, float dt);
    
    private:
        std::vector<ContactConstraint> currentFrameContacts;
        std::vector<ContactConstraint> lastFrameContacts;

        std::unordered_map<unsigned int, ImpulseCache> impulseCache;
};