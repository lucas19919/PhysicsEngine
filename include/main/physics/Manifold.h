#pragma once
#include "main/physics/SAT.h"

struct CollisionManifold {
    Collision Collision;
    std::vector<Vec2> points;
};

struct Edge {
    Vec2 p1;
    Vec2 p2;
    Vec2 line;
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
        static std::vector<Vec2> GetVertices(Collider* c);
        static std::vector<Edge> GetEdges(std::vector<Vec2> vertices);

        static Edge GetSupportFace(std::vector<Vec2> vertices, Vec2 normal);


        static std::vector<Vec2> GetPolygonContacts(std::vector<Vec2> vertices1, std::vector<Vec2> vertices2, Vec2 normal);
        static std::vector<Vec2> GetPolygonCircleContacts(std::vector<Vec2> vertices1, Vec2 center);
};