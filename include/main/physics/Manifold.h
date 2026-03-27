#pragma once
#include "main/physics/SAT.h"

struct CollisionManifold {
    Collision Collision;
    std::vector<Vec2> points;
};

class Manifold
{
    public:
        static CollisionManifold GenCircleCircle(Collider* c1, Collider* c2);
        static CollisionManifold GenBoxCircle(Collider* c1, Collider* c2);
        static CollisionManifold GenBoxBox(Collider* c1, Collider* c2);
        static CollisionManifold GenPolyCircle(Collider* c1, Collider* c2);
        static CollisionManifold GenPolyBox(Collider* c1, Collider* c2);
        static CollisionManifold GenPolyPoly(Collider* c1, Collider* c2);

    private:
        //helper functions
};