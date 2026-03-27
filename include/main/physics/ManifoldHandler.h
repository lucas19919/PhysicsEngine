#pragma once
#include "main/physics/Manifold.h"
#include "main/GameObject.h"

class ManifoldHandler
{
    public:
        static CollisionManifold SortManifold(GameObject* obj1, GameObject* obj2);
};