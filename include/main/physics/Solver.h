#pragma once
#include "main/physics/Manifold.h"
#include "main/physics/ManifoldHandler.h"
#include "main/World.h"

class GameObject;
class ContactConstraint;

class Solver
{
    public: 
        static CollisionManifold ResolveManifold(GameObject* obj1, GameObject* obj2);
        static void ResolveConstraints(ContactConstraint& contact);
        static void ResolvePosition(ContactConstraint& contact);
        static void Warmstart(ContactConstraint& contact);
};