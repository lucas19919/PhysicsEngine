#pragma once
#include <vector>
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/collidertypes/PolygonCollider.h"

struct Manifold {
    bool isColliding;
    Vec2 normal;
    float depth;
    std::vector<Vec2> contactPoints;
};

class SAT
{
    private:
        struct Projection {
            float min;
            float max;
        };

        static Projection Project(std::vector<Vec2> vertices, const Vec2 axis);
        static Projection CircleProject(CircleCollider* c, const Vec2 axis);
        static std::vector<Vec2> GetVertices(Collider* b);
        static std::vector<Vec2> GetNormals(std::vector<Vec2> vertices);

    public:
        static Manifold CircleCircle(Collider* c1, Collider* c2);
        static Manifold BoxBox(Collider* b1, Collider* b2);
        static Manifold BoxCircle(Collider* b1, Collider* c1);
        static Manifold PolygonCircle(Collider* p1, Collider* c1);
        static Manifold PolygonBox(Collider* p1, Collider* b1);
        static Manifold PolygonPolygon(Collider* p1, Collider* p2);
};