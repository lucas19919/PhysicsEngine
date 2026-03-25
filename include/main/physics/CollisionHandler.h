#pragma once
#include "main/physics/SAT.h"
#include "main/components/Collider.h"

class CollisionHandler
{
    public:
        static Manifold SortCollision(Collider* c1, Collider* c2);
};