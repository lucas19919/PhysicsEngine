#pragma once
#include "main/GameObject.h"
#include "main/physics/Manifold.h"

class ManifoldHandler
{
    public:
        static CollisionManifold SortManifold(GameObject* obj1, GameObject* obj2);
};