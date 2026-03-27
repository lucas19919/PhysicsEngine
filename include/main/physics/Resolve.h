#pragma once
#include "main/physics/Manifold.h"
#include "main/physics/ManifoldHandler.h"

class GameObject;

class Resolve
{
    public: 
        static CollisionManifold ResolveManifold(GameObject* obj1, GameObject* obj2);
        static void ResolveImpulse(CollisionManifold manifold, GameObject* obj1, GameObject* obj2);
        static void ResolvePosition(CollisionManifold manifold, GameObject* obj1, GameObject* obj2);
};