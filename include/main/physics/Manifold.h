#pragma once
#include "main/physics/SAT.h"
#include "main/utility/templates/Array.h"

struct CollisionManifold {
    Collision collision;
    Array<4> points;
};

struct Edge {
    Vec2 p1;
    Vec2 p2;
    Vec2 line;
};

class Manifold
{
    public:
        static CollisionManifold GenCircleCircle(GameObject* obj1, GameObject* obj2);
        static CollisionManifold GenBoxCircle(GameObject* obj1, GameObject* obj2);
        static CollisionManifold GenBoxBox(GameObject* obj1, GameObject* obj2);
        static CollisionManifold GenPolyCircle(GameObject* obj1, GameObject* obj2);
        static CollisionManifold GenPolyBox(GameObject* obj1, GameObject* obj2);
        static CollisionManifold GenPolyPoly(GameObject* obj1, GameObject* obj2);

    private:
        static Array<20> GetVertices(GameObject* obj);
        static Edge GetSupportFace(const Array<20>& vertices, Vec2 normal);

        static Array<4> GetPolygonContacts(const Array<20>& vertices1, const Array<20>& vertices2, Vec2 normal);
        static Array<4> GetPolygonCircleContacts(const Array<20>& vertices1, Vec2 center);
};