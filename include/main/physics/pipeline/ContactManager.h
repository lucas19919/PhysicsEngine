#pragma once
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/components/RigidBody.h"
#include "main/utility/templates/Array.h"
#include "main/components/Constraint.h"

#include <vector>
#include <memory>

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
};